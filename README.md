# Nyxty Engine

Nyxty is a modular, data-oriented C++20 game engine designed for performance and extensibility. It provides a foundational core platform upon which standalone applications and games can be built.

## Core Concepts

The engine is built around a few key principles:

-   **Modularity:** Features are organized into distinct modules (`core`, `rendering`, `services`, etc.) that can be managed by a central module system. This allows for a clean separation of concerns and makes the engine highly extensible.
-   **Data-Oriented Design:** The engine favors data-oriented patterns over traditional object-oriented hierarchies, particularly in performance-critical systems. This is most evident in the design of the event system.
-   **Layered Architecture:** The engine is structured in layers, from the low-level `platform` and `core` libraries to higher-level `services` and the final `application` layer.

## The Event System

A core feature of Nyxty's architecture is its powerful and efficient event system. It is designed to allow different engine modules to communicate with each other without being directly dependent on one another, a concept known as decoupling.

### Event System Flow

The flow is straightforward: a system **dispatches** an event, and any other system that has **subscribed** to that event type will have its callback function invoked.

1.  **Event Definition:** An `Event` is a simple, fixed-size data structure (a POD or "Plain Old Data" type). It contains a type identifier, a timestamp, and a raw data buffer. This design is intentional for performance: events can be safely copied with `memcpy`, have a predictable memory layout, and don't require dynamic memory allocation.

    *See `engine/core/events/Event.h` and `engine/core/events/EventTypes.h`.*

    ```cpp
    // A simple data structure for a key press
    struct KeyPressEventData {
        u32 windowID;
        i32 keyCode;
        // ...
    };

    // Somewhere in the input system...
    // Create an event with the data
    Event keyEvent = Event::Create(EVENT_KEY_PRESS, KeyPressEventData{...});
    ```

2.  **Dispatching:** The `EventBus` is a singleton that acts as the central message dispatcher. When a module wants to signal that something has happened, it dispatches an event through the `EventBus`.

    *See `engine/core/events/EventBus.h`.*

    ```cpp
    // Dispatch the event immediately to all listeners
    EventBus::GetInstance().Dispatch(keyEvent);

    // Or, queue it to be dispatched at a later, controlled time
    EventBus::GetInstance().Queue(keyEvent);
    ```

3.  **Subscribing (Listening):** Any other system (e.g., the UI, a game script) can subscribe to specific event types. A subscriber provides a callback function that will be executed when an event of that type is dispatched. The `EventBus` uses `entt::delegate`, a high-performance, allocation-free delegate system, to store these callbacks.

    ```cpp
    // A listener function in a UI module
    void OnKeyPress(const Event& event) {
        if (const auto* keyData = event.As<KeyPressEventData>()) {
            // Handle the key press...
            Log::Info("Key {0} was pressed!", keyData->keyCode);
        }
    }

    // In the UI module's initialization...
    // Subscribe the listener to the key press event type
    EventBus::GetInstance().Subscribe(EVENT_KEY_PRESS, {&OnKeyPress});
    ```

This decoupled system means the input system doesn't need to know anything about the UI system. It simply dispatches an event, and any part of the engine that cares about that event can react to it.

## Getting Started

### Build Instructions

To build the project, you can use the provided CMake presets from the root of the repository.

```powershell
# 1. Configure the project (generates build files)
cmake --preset msvc-x64-debug

# 2. Build the project
cmake --build --preset build-msvc-x64-debug

# 3. Run the main application (the Hub)
.\out\build\msvc-x64-debug\bin\Debug\nyxty_hub.exe
```

## Project Structure

The repository is organized into several key directories:

-   `engine/`: Contains the core engine modules.
    -   `core/`: The lowest-level, foundational systems (memory, events, platform abstraction).
    -   `services/`: Higher-level engine systems (windowing, assets, serialization).
    -   `rendering/`, `scripting/`, `ui/`: Other major engine components.
-   `runtime/`: Provides the application framework (`IApp`, `AppRunner`) and the module loading system.
-   `apps/`: Standalone executables built on the engine. `apps/hub` is the primary development and editor application.
-   `third_party/`: External libraries and dependencies like SDL3, ImGui, and EnTT.
-   `docs/`: Engine documentation.
