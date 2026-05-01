# Nyxty

Nyxty is the core platform. Apps are standalone systems built on that core.

## Getting Started

Build from the repo root:

```powershell
cmake --preset msvc-x64-debug
cmake --build --preset build-msvc-x64-debug
.\out\build\msvc-x64-debug\bin\Debug\nyxty_hub.exe
```

## Documentation

For a deeper understanding of the engine's architecture and systems, please refer to the documentation files in the root of the project:

-   `ARCHITECTURE.md`: High-level overview of the engine's design.
-   `BUILD_SYSTEM.md`: Explanation of the CMake build process.
-   `CORE_ENGINE.md`: Deep dive into the foundational engine components.
-   `RUNTIME.md`: How the application framework and module system work.
-   `EVENT_SYSTEM.md`: Detailed explanation of the event bus and event flow.
