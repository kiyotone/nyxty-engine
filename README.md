# Nyxty

Nyxty is the core platform. Apps are standalone systems built on that core.

Current shape:

```text
nyxty/
  engine/
    core/
      foundation/
      memory/
      handles/
      time/
      logging/
      events/
      platform/
    services/
      assets/
      config/
      container/
      filesystem/
      notifications/
      project/
      serialization/
      tasks/
      window/
    rendering/
      backend/
    ui/
    scripting/
      runtime/

  runtime/
    app_framework/
      IApp.h
      AppRunner.h
      AppRunner.cpp
    module_system/
      IModule.h
      ModuleLoader.h
      ModuleLoader.cpp

  apps/
    hub/
      HubApp.h
      HubApp.cpp
      main.cpp
      tray/
      ui/
      modules/
        notes/
        terminal/
        search/
        profiler/
    nyxel/

  third_party/
  docs/
  projects/
  tools/
```

`apps/hub` is the first host app. It boots the shared core services and opens the engine inspector. `apps/nyxel` is reserved for the future standalone voxel app.

`runtime/module_system` holds the reusable module loader and plugin interfaces.

`apps/hub/modules` holds hub-specific feature modules that build as runtime DLLs and are loaded by the hub from `apps/hub/modules` under the runtime output.

Current modules:

- `notes`: session scratchpad with copy/clear actions.
- `terminal`: script-host command runner.
- `search`: command palette helper actions.
- `profiler`: lightweight runtime update stats.

`apps/hub` also adds a Windows system tray icon with show, hide-to-tray, toggle, and quit actions.

`third_party` owns vendored dependencies such as SDL3, ImGui, bgfx.cmake, glm, spdlog, EnTT, and json.

`engine/core` is the bottom layer. Systems that depend on it live in `services`, `rendering`, and `scripting`:

- `core/memory/`: linear, stack, and fixed-block allocators.
- `core/time/`: frame time, fixed timestep value, frame count, and scaled time.
- `core/handles/`: typed generational handles.
- `services/serialization/`: JSON file load/save/validation service.
- `services/notifications/`: user-facing notification center, separate from logs.
- `scripting/runtime/`: lightweight command hook layer for future scripting backends.
- `services/project/`: `.nyxty` project loading/saving.
- `services/assets/`: asset pipeline job planning and simple copy/validation rules.
- `services/config/`: debug/release/shipping profile metadata.

`runtime/app_framework` provides a tiny host app runner contract (`IApp` + `AppRunner`) so the executable entrypoint stays thin.

Build from the repo root:

```powershell
cmake --preset msvc-x64-debug
cmake --build --preset build-msvc-x64-debug
.\out\build\msvc-x64-debug\bin\Debug\nyxty_hub.exe
```
