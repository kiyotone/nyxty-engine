# Nyxty Roadmap

## Phase 1: Core Platform
- Keep the engine small and reusable: app loop, windowing, rendering, input, logging, events, files, tasks, config, and services.
- Keep `engine/core` as the bottom layer; keep serialization, notifications, project settings, asset pipeline, and build profiles in `engine/services`.
- Build core systems only when the current app needs them.
- Keep app-specific behavior out of the engine.

## Phase 2: NyxtyHub
- Use `apps/hub` as the first host app.
- Keep the hub focused on core inspection and shared workspace services for now.
- Keep reusable module plumbing in `runtime/module_system` and keep hub-specific modules under `apps/hub/modules`.

## Phase 3: Reintroduce Modules Deliberately
- Prototype small tools as modules when the hub needs fast add/remove workflows.
- Good first module candidates: Debug Overlay and Console.
- Keep module dependencies shallow and explicit.
- Extract shared behavior into the engine only after reuse appears in more than one place.

## Phase 4: Promote Mature Systems
- Move large, specialized tools into standalone apps under `apps/`.
- Keep shared systems in `engine/`.
- Keep optional developer utilities under `tools/` when build-time workflows appear.
