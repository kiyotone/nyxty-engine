# nyxty-engine

A C++20 modular host runtime. Designed to be consumed as a **git submodule** — not run directly.

Built to power diverse long-lived applications: editors, renderers, simulations, browsers. Each capability domain is a hot-reloadable DLL module; modules are assembled into applications without touching the engine core.

**Current version:** v2.0.0

---

## Architecture

```
core/           ← core.dll — foundation, events, logging, jobs, services, module loader
interfaces/     ← header-only abstract C++ interfaces           (planned)
modules/        ← concrete capability DLLs (SDL3, bgfx, …)     (planned)
applications/   ← apps assembled from chosen modules            (planned)
tools/          ← tooling                                       (planned)
```

Dependency rule (never reversed): **applications → modules → interfaces → core → OS**

`core.dll` has zero SDL3 / bgfx imports. Those live exclusively in module DLLs.

---

## Third-party dependencies

| Library | Purpose |
|---|---|
| [glm](https://github.com/g-truc/glm) | Math |
| [spdlog](https://github.com/gabime/spdlog) | Logging backend |
| [EnTT](https://github.com/skypjack/entt) | ECS / delegates |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON config / manifests |
| [SDL3](https://github.com/libsdl-org/SDL) | Window / input (modules only) |
| [bgfx](https://github.com/bkaradzic/bgfx) | Rendering (modules only) |
| [Dear ImGui](https://github.com/ocornut/imgui) | UI (modules only) |

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

## IModule interface

Modules are hot-reloadable DLLs. Each module implements `IModule` and exports two factory functions.

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

    bool OnLoad()   override { /* init */    return true; }
    void OnUnload() override { /* cleanup */ }
    void OnUpdate() override { /* per-frame */ }
};

MODULE_EXPORT IModule* CreateModule()            { return new MyModule(); }
MODULE_EXPORT void     DestroyModule(IModule* m) { delete m; }
```

`MODULE_EXPORT` expands to `extern "C" __declspec(dllexport)` on Windows and `extern "C" __attribute__((visibility("default")))` elsewhere.

### Optional extensions

`IModule` has two optional surfaces for editor/tool integration:

- **Inspector panels** — `GetInspectorPanels()` / `ApplyInspectorField()` expose per-module UI panels.
- **Command palette** — `GetPaletteItems()` / `InvokePaletteItem()` feed a searchable action palette.

Both default to no-ops; override only if your module needs tool UI.

---

## IApp / AppRunner

```cpp
#include "core/runtime/IApp.h"
#include "core/runtime/AppRunner.h"
#include "core/runtime/ModuleLoader.h"
#include "core/logging/Log.h"

class MyApp : public Nyxty::IApp {
public:
    int Run() override {
        Nyxty::Log::Init();

        m_Loader.DiscoverModules(BIN_DIR);   // loads .dll + .module.json pairs
        m_Loader.StartWatcher(BIN_DIR);       // enables hot-reload

        while (m_Running) {
            m_Loader.ProcessPendingReloads();
            for (auto* mod : m_Loader.GetModules()) mod->OnUpdate();
        }

        m_Loader.StopWatcher();
        m_Loader.Unload();
        return 0;
    }
private:
    Nyxty::ModuleLoader m_Loader;
    bool m_Running = true;
};

int main() {
    MyApp app;
    return Nyxty::AppRunner{}.Run(app);
}
```

Hot-reload: rebuild the DLL while the app is running — `FileWatcher` detects the change and queues a reload via `ProcessPendingReloads()`.

---

## ServiceLocator

Register and resolve services by string key and type:

```cpp
#include "core/services/ServiceLocator.h"

// Register
Nyxty::ServiceLocator::Register<IRenderer>("renderer", &myRenderer);

// Resolve
auto* r = Nyxty::ServiceLocator::Get<IRenderer>("renderer");

// Cleanup
Nyxty::ServiceLocator::Clear();
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

No apps are built from the engine root. Apps are standalone projects that consume this repo as a submodule.

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
