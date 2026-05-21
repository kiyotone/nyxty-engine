# nyxty-engine

A C++20 modular game engine core. Designed to be consumed as a **git submodule** by standalone app projects — not run directly.

**Current version:** v1.3.0

---

## What it provides

| Module | Type | Purpose |
|---|---|---|
| `platform` | DLL | Win32 / SDL3 window and input abstraction |
| `core` | DLL | Logging, ECS, event bus, service locator |
| `engine` | INTERFACE | Aggregates headers from platform + core |
| `app_framework` | STATIC | `IApp` interface + `AppRunner` |
| `modsys` | DLL | `ModuleLoader` — hot-reloadable DLL modules |

---

## Using as a submodule

```powershell
# In your app repo:
git submodule add https://github.com/kiyotone/nyxty-engine.git engine
```

Root `CMakeLists.txt` of your app:
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp LANGUAGES CXX)

add_subdirectory(engine)   # builds platform, core, modsys, app_framework

add_subdirectory(app)
add_subdirectory(modules/my_module)
```

Link your targets:
```cmake
target_link_libraries(my_app  PRIVATE app_framework modsys core engine)
target_link_libraries(my_mod  PRIVATE modsys core engine)
```

---

## Module system

Modules are hot-reloadable DLLs. Each module:

1. Implements `IModule` (OnLoad / OnUpdate / OnUnload)
2. Exports `CreateModule()` and `DestroyModule()` factory functions
3. Ships a `.module.json` manifest so `ModuleLoader` can discover it

```cpp
#include "engine/core/foundation/Base.h"       // MODULE_EXPORT
#include "runtime/module_system/IModule.h"

class MyModule : public IModule {
public:
    const char*  GetName()         override { return "my_module"; }
    Nyxty::u32   GetVersionMajor() override { return 1; }
    Nyxty::u32   GetVersionMinor() override { return 0; }
    Nyxty::u32   GetVersionPatch() override { return 0; }
    Nyxty::u32   GetModuleID()     override { return 0x00000001; }
    bool         OnLoad()          override { /* init */ return true; }
    void         OnUnload()        override { /* cleanup */ }
    void         OnUpdate()        override { /* per-frame */ }
};

MODULE_EXPORT IModule* CreateModule()         { return new MyModule(); }
MODULE_EXPORT void     DestroyModule(IModule* m) { delete m; }
```

```json
{
    "id": "my_module",
    "name": "My Module",
    "version": "1.0.0",
    "library": "my_module.dll",
    "load_priority": 0,
    "enabled": true,
    "dependencies": []
}
```

Hot-reload: just rebuild the DLL while the app is running. The `FileWatcher` detects the change and reloads on the next frame.

---

## App skeleton

```cpp
#include "runtime/app_framework/IApp.h"
#include "runtime/module_system/ModuleLoader.h"
#include "engine/rendering/backend/Renderer.h"
#include "engine/services/window/Window.h"
#include "engine/services/container/ServiceLocator.h"

class MyApp : public Nyxty::IApp {
public:
    int Run() override {
        Nyxty::Log::Init();
        Nyxty::Window::InitSDL();
        m_Window.Init({ .title = "MyApp", .width = 1280, .height = 720 });
        m_Renderer.Init({ .nwh = m_Window.GetNativeHandle(), .width = 1280, .height = 720 });
        Nyxty::ServiceLocator::Provide(&m_Window, &m_Renderer, nullptr);

        m_Loader.DiscoverModules(BIN_DIR);
        m_Loader.StartWatcher(BIN_DIR);

        while (!m_Window.ShouldClose()) {
            Nyxty::Window::PollEvents();
            m_Loader.ProcessPendingReloads();
            m_Renderer.BeginFrame(0);
            for (auto* mod : m_Loader.GetModules()) mod->OnUpdate();
            m_Renderer.EndFrame();
        }

        m_Loader.StopWatcher();
        m_Loader.UnloadAll();
        Nyxty::ServiceLocator::Clear();
        m_Renderer.Shutdown();
        m_Window.Shutdown();
        Nyxty::Window::ShutdownSDL();
        return 0;
    }
private:
    Nyxty::Window       m_Window;
    Nyxty::Renderer     m_Renderer;
    Nyxty::ModuleLoader m_Loader;
};

int main() {
    return Nyxty::AppRunner{}.Run(MyApp{});
}
```

---

## Build (standalone, for engine development)

```powershell
cmake -S . -B build
cmake --build build
```

No apps are built from the engine root. Apps are standalone projects that consume this repo as a submodule.

---

## Updating the submodule in a consumer app

```powershell
# In your app repo:
cd engine && git pull
cd ..
git add engine
git commit -m "chore: bump engine to vX.Y.Z"
```

---

## Apps using this engine

| App | Description |
|---|---|
| [nyxel](https://github.com/kiyotone/nyxel) | GPU voxel rendering lab — DDA ray-marcher, hot-reloadable trace modules |
