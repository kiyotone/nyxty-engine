# nyxty-engine

A C++20 modular host runtime. Designed to be consumed as a **git submodule** — not run directly.

Built to power diverse long-lived applications: editors, renderers, simulations, browsers. Each capability domain is a hot-reloadable DLL module; modules are assembled into applications without touching the engine core.

**Current version:** v2.0.0

---

## Folder structure

```
nyxty-engine/
├── core/                        ← core.dll — the only build target in this repo
│   ├── foundation/              ← Base types, type aliases, DLL macros, debug break
│   │   ├── Base.h               ← Scope<T>/Ref<T>, integer aliases, MODULE_EXPORT
│   │   ├── Core.h               ← NYXTY_CORE_API / NYXTY_MODULE_API dllexport macros
│   │   └── Debug.h
│   ├── platform/                ← OS detection
│   │   ├── Platform.h
│   │   └── OS.h / OS.cpp
│   ├── events/                  ← Zero-allocation event bus (EnTT delegates)
│   │   ├── EventTypes.h         ← Enum: system(1-99), input(100-199), window(200-299), renderer(300-399), asset(400-499), custom(1000+)
│   │   ├── Event.h              ← POD Event struct (256-byte inline data, no heap)
│   │   ├── EventBus.h/.cpp      ← Subscribe / Dispatch / Queue / DispatchQueued
│   │   ├── EventBusInstance.h   ← Global singleton accessor
│   │   └── EventTypes.cpp
│   ├── logging/                 ← spdlog wrapper with per-module loggers + ring-buffer sink
│   │   ├── Log.h                ← NYXTY_ENGINE_*/NYXTY_APP_*/NYXTY_MODULE_* macros
│   │   ├── Log.cpp
│   │   └── LogSink.h            ← RingBufferSink for in-app log panels
│   ├── memory/                  ← Custom allocators (no STL heap overhead)
│   │   └── Memory.h/.cpp        ← LinearAllocator, StackAllocator, FixedBlockAllocator
│   ├── handles/                 ← Generational handle system
│   │   └── Handle.h             ← Handle<Tag> + HandlePool<Tag>
│   ├── time/
│   │   └── TimeSystem.h/.cpp
│   ├── jobs/                    ← Multi-threaded task system
│   │   └── TaskSystem.h/.cpp    ← Worker pool + main-thread drain queue; Submit<Fn>() returns std::future
│   ├── config/                  ← JSON-backed runtime config
│   │   ├── Config.h/.cpp        ← Singleton; dot-path Get<T>/Set<T>; fires EVENT_CONFIG_CHANGED
│   │   └── BuildProfile.h/.cpp
│   ├── io/
│   │   ├── FileWatcher.h/.cpp   ← Background thread; fires callback on file change (powers hot-reload)
│   │   └── SerializationService.h/.cpp  ← JSON load/save/validate helpers
│   ├── services/                ← Runtime service registry
│   │   ├── ServiceLocator.h     ← Thread-safe Register<T>/Get<T>/GetAll<T> by string key
│   │   ├── ServiceLocator.cpp
│   │   ├── NotificationCenter.h/.cpp  ← Push/dismiss timed notifications (Info/Success/Warning/Error)
│   │   ├── AssetPipeline.h/.cpp ← Rule-based asset copy/validate/compile planner
│   │   └── Project.h/.cpp       ← Active ProjectSettings (name, paths, module list, build profile)
│   ├── reflection/
│   │   └── TypeRegistry.h       ← type_index → name map; placeholder for full reflection
│   └── runtime/                 ← App lifecycle + module management
│       ├── IApp.h               ← Pure interface: virtual int Run()
│       ├── AppRunner.h/.cpp     ← Wraps IApp::Run(); entry point shim
│       ├── IModule.h            ← Pure interface every module DLL implements
│       │                           GetName / GetVersion / GetModuleID
│       │                           OnLoad / OnUnload / OnUpdate
│       │                           GetInspectorPanels / ApplyInspectorField (optional UI)
│       │                           GetPaletteItems / InvokePaletteItem (optional command palette)
│       │                           GetPanelName / OnDrawUI (optional debug panel)
│       └── ModuleLoader.h/.cpp  ← DiscoverModules / Load / Reload / Unload
│                                    StartWatcher / StopWatcher / ProcessPendingReloads
│
├── third_party/                 ← Git submodules (vendored, not built by this repo's CMake)
│   ├── spdlog/                  ← Logging backend
│   ├── entt/                    ← ECS + zero-alloc delegates (EventBus uses entt::delegate)
│   ├── glm/                     ← Math (only third_party built by root CMakeLists.txt)
│   ├── json/                    ← nlohmann/json (header-only, Config + SerializationService)
│   ├── imgui/                   ← Dear ImGui (module-only; not linked by core)
│   ├── SDL3/                    ← Window/input (module-only; not linked by core)
│   └── bgfx.cmake/              ← bgfx rendering (module-only; not linked by core)
│
├── CMakeLists.txt               ← Root: C++20, platform defines, adds glm + core
└── README.md
```

**Planned (not yet present):**

```
interfaces/     ← header-only abstract C++ interfaces shared between modules
modules/        ← concrete capability DLLs (SDL3 window, bgfx renderer, …)
applications/   ← apps assembled from chosen modules
tools/          ← tooling
```

Dependency rule (never reversed): **applications → modules → interfaces → core → OS**

`core.dll` has zero SDL3 / bgfx imports. Those live exclusively in module DLLs.

---

## Architecture & data flow

```
main()
  └─ AppRunner::Run(IApp&)
       └─ IApp::Run()
            ├─ Log::Init()                        ← spdlog engine + app loggers, RingBufferSink
            ├─ Config::Instance().Load(path)       ← JSON config into dot-path tree
            ├─ TaskSystem::Start(workerCount)      ← spin up worker threads
            ├─ ModuleLoader::DiscoverModules(dir)  ← scan for .dll + .module.json pairs
            │    └─ for each DLL:
            │         dlopen / LoadLibrary
            │         CreateModule() → IModule*
            │         IModule::OnLoad()
            │         EventBus::Dispatch(EVENT_MODULE_LOADED)
            ├─ ModuleLoader::StartWatcher(dir)     ← FileWatcher background thread
            │
            └─ [game loop]
                 ├─ ModuleLoader::ProcessPendingReloads()
                 │    └─ if DLL changed on disk:
                 │         IModule::OnUnload()
                 │         EventBus::UnsubscribeAll(moduleID)
                 │         dlclose / FreeLibrary
                 │         copy new .dll → live path
                 │         dlopen → CreateModule() → IModule::OnLoad()
                 ├─ EventBus::DispatchQueued()      ← flush deferred events
                 ├─ TaskSystem::DrainMainThread()   ← run tasks posted from workers
                 └─ for each IModule*: OnUpdate()
```

### Event system

`EventBus` is synchronous and zero-allocation. Callbacks are `entt::delegate<void(const Event&)>` — no `std::function` heap overhead.

```
EventBus::Subscribe(type, callback, moduleID)
EventBus::Dispatch(event)          ← immediate; calls all handlers inline
EventBus::Queue(event)             ← deferred; replayed by DispatchQueued()
EventBus::UnsubscribeAll(moduleID) ← called automatically on hot-reload
```

`Event` carries up to 256 bytes of inline POD data (no pointers, no heap). Access typed payload with `event.As<MyData>()`.

### Service locator

```cpp
// Register a service (e.g. in a module's OnLoad):
Nyxty::ServiceLocator::Register<IRenderer>("renderer", &myRenderer);

// Resolve from anywhere:
auto* r = Nyxty::ServiceLocator::Get<IRenderer>("renderer");

// Get all services of a type:
auto renderers = Nyxty::ServiceLocator::GetAll<IRenderer>();

// Cleanup (e.g. on module unload):
Nyxty::ServiceLocator::Unregister("renderer");
```

### Memory allocators

`core/memory/Memory.h` provides three arena-style allocators for hot paths that must avoid heap fragmentation:

| Allocator | Use case |
|---|---|
| `LinearAllocator` | Frame scratch memory; Reset() reclaims all at once |
| `StackAllocator` | Nested scopes; FreeToMarker() rolls back to a saved point |
| `FixedBlockAllocator` | Fixed-size object pools; O(1) alloc + free |

### Handle system

`Handle<Tag>` is a generational index (index + generation u32 pair). `HandlePool<Tag>` tracks live/dead handles and reuses indices safely. Use it to safely reference pooled objects without dangling pointers.

---

## Third-party dependencies

| Library | Purpose | Linked by |
|---|---|---|
| [glm](https://github.com/g-truc/glm) | Math | core (public) |
| [spdlog](https://github.com/gabime/spdlog) | Logging backend | core (header-only include) |
| [EnTT](https://github.com/skypjack/entt) | Zero-alloc delegates for EventBus | core (header-only include) |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON config + serialization | core (header-only include) |

---

## Using as a submodule

```powershell
# In your app repo:
git submodule add https://github.com/kiyotone/nyxty-engine.git engine
git submodule update --init --recursive
```

Root `CMakeLists.txt` of your app:

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp LANGUAGES CXX)

set(NYXTY_ROOT_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/engine" CACHE PATH "")
set(NYXTY_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/third_party" CACHE PATH "")

add_subdirectory(engine/third_party/glm ${CMAKE_BINARY_DIR}/third_party/glm)
add_subdirectory(engine)   # builds core.dll

add_subdirectory(app)
add_subdirectory(modules/my_module)
```

Link your targets:

```cmake
target_link_libraries(my_app  PRIVATE core)
target_link_libraries(my_mod  PRIVATE core)
```

---

## Writing a module

Every module DLL implements `IModule` and exports two C factory functions.

```cpp
#include "core/runtime/IModule.h"   // IModule
#include "core/foundation/Base.h"   // MODULE_EXPORT, Nyxty::u32

class MyModule : public IModule {
public:
    const char* GetName()         override { return "my_module"; }
    Nyxty::u32  GetVersionMajor() override { return 1; }
    Nyxty::u32  GetVersionMinor() override { return 0; }
    Nyxty::u32  GetVersionPatch() override { return 0; }
    Nyxty::u32  GetModuleID()     override { return 0x00000001; }

    bool OnLoad() override {
        // Subscribe to events, register services, allocate resources
        auto& bus = Nyxty::EventBusInstance::Get();
        bus.Subscribe(Nyxty::EVENT_APP_UPDATE, NYXTY_BIND_EVENT_FN(OnUpdate_Event), GetModuleID());
        return true;
    }

    void OnUnload() override {
        // EventBus::UnsubscribeAll is called automatically by ModuleLoader
    }

    void OnUpdate() override { /* per-frame logic */ }

private:
    void OnUpdate_Event(const Nyxty::Event& e) { /* handle event */ }
};

MODULE_EXPORT IModule* CreateModule()            { return new MyModule(); }
MODULE_EXPORT void     DestroyModule(IModule* m) { delete m; }
```

`MODULE_EXPORT` expands to `extern "C" __declspec(dllexport)` on Windows and `extern "C" __attribute__((visibility("default")))` elsewhere.

The module JSON manifest (placed alongside the DLL) tells the loader metadata:

```json
{
  "name": "my_module",
  "id": "0x00000001",
  "version": "1.0.0"
}
```

### Optional module extensions

| Override | Purpose |
|---|---|
| `GetInspectorPanels()` / `ApplyInspectorField()` | Per-module editor inspector UI panels |
| `GetPaletteItems()` / `InvokePaletteItem()` | Searchable command palette entries |
| `GetPanelName()` / `OnDrawUI()` | Docked debug UI panel drawn each frame |

All default to no-ops; override only what you need.

### Logging from a module

```cpp
NYXTY_MODULE_INFO("my_module", "Loaded with {} meshes", count);
NYXTY_MODULE_WARN("my_module", "Asset not found: {}", path);
```

---

## Writing an app

```cpp
#include "core/runtime/IApp.h"
#include "core/runtime/AppRunner.h"
#include "core/runtime/ModuleLoader.h"
#include "core/logging/Log.h"
#include "core/config/Config.h"
#include "core/jobs/TaskSystem.h"
#include "core/events/EventBusInstance.h"

class MyApp : public Nyxty::IApp {
public:
    int Run() override {
        Nyxty::Log::Init();
        Nyxty::Config::Instance().Load("config.json");
        m_Tasks.Start();                              // worker threads = std::thread::hardware_concurrency

        m_Loader.DiscoverModules(BIN_DIR);            // loads .dll + .module.json pairs
        m_Loader.StartWatcher(BIN_DIR);               // enables hot-reload

        while (m_Running) {
            m_Loader.ProcessPendingReloads();          // swap DLLs that changed on disk
            Nyxty::EventBusInstance::Get().DispatchQueued();
            m_Tasks.DrainMainThread();
            for (auto* mod : m_Loader.GetModules()) mod->OnUpdate();
        }

        m_Loader.StopWatcher();
        m_Loader.Unload();
        m_Tasks.Stop();
        return 0;
    }

private:
    Nyxty::ModuleLoader m_Loader;
    Nyxty::TaskSystem   m_Tasks;
    bool m_Running = true;
};

int main() {
    MyApp app;
    return Nyxty::AppRunner{}.Run(app);
}
```

---

## Build (standalone — engine development only)

```powershell
# Using CMake presets (recommended):
cmake --preset msvc-x64-debug
cmake --build --preset build-msvc-x64-debug

# Or manually:
cmake -S . -B build
cmake --build build
```

Output: `build/bin/core.dll` (+ import lib).

No apps are built from the engine root. Apps are standalone CMake projects that `add_subdirectory(engine)`.

---

## Updating the submodule in a consumer app

```powershell
cd engine
git pull
cd ..
git add engine
git commit -m "chore: bump engine submodule"
```

---

## Apps using this engine

| App | Description |
|---|---|
| [nyxel](https://github.com/kiyotone/nyxel) | GPU voxel rendering lab |