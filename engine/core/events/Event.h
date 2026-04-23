#pragma once

#include "core/events/EventTypes.h"
#include "core/foundation/Base.h"
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <type_traits>
#include "core/foundation/Core.h"

namespace Nyxty {

    // Maximum size for event data (256 bytes)
    static constexpr size_t EVENT_DATA_SIZE = 256;

    // ========================================
    // Base Event Structure (POD - Plain Old Data)
    // ========================================
    // Rules:
    // - NO pointers
    // - NO std::string, std::vector, etc.
    // - NO virtual functions
    // - Must be memcpy-safe (trivially copyable)

    struct NYXTY_CORE_API  Event {
        EventType type = EVENT_NONE;      // What kind of event is this?
        u32 size = 0;                     // Size of actual data stored
        u64 timestamp = 0;                // When did this event occur?
        u32 sourceModuleID = 0;           // Which module sent this event?

        // Raw data storage (POD only!)
        alignas(8) char data[EVENT_DATA_SIZE];

        // ========================================
        // Factory: Create Empty Event
        // ========================================
        template<typename T>
        static Event Create(EventType eventType, u32 moduleID = 0) {
            static_assert(sizeof(T) <= EVENT_DATA_SIZE, "Event data too large!");
            static_assert(std::is_trivially_copyable_v<T>, "Event data must be POD/trivially copyable!");

            Event evt;
            evt.type = eventType;
            evt.size = sizeof(T);
            evt.sourceModuleID = moduleID;
            evt.timestamp = GetTimestamp();
            return evt;
        }

        // ========================================
        // Factory: Create Event With Data
        // ========================================
        template<typename T>
        static Event Create(EventType eventType, const T& eventData, u32 moduleID = 0) {
            Event evt = Create<T>(eventType, moduleID);
            std::memcpy(evt.data, &eventData, sizeof(T));
            return evt;
        }

        // ========================================
        // Access Typed Data (Read-Only)
        // ========================================
        template<typename T>
        const T* As() const {
            if (size != sizeof(T)) {
                return nullptr;  // Size mismatch
            }
            return reinterpret_cast<const T*>(data);
        }

        // ========================================
        // Access Typed Data (Mutable)
        // ========================================
        template<typename T>
        T* As() {
            if (size != sizeof(T)) {
                return nullptr;  // Size mismatch
            }
            return reinterpret_cast<T*>(data);
        }

    private:
        static u64 GetTimestamp();
    };

    // ========================================
    // Example POD Event Data Structures
    // ========================================

    struct NYXTY_CORE_API KeyPressEventData {
        u32 windowID;
        i32 keyCode;
        u32 modifiers;
        bool isPressed;
        bool isRepeat;
    };

    struct NYXTY_CORE_API WindowEventData {
        u32 windowID;
    };

    struct NYXTY_CORE_API WindowMoveEventData {
        u32 windowID;
        i32 x;
        i32 y;
    };

    struct NYXTY_CORE_API WindowResizeEventData {
        u32 windowID;
        i32 width;
        i32 height;
        f32 dpiScale;
    };

    struct NYXTY_CORE_API MouseMoveEventData {
        u32 windowID;
        f32 x;
        f32 y;
        f32 deltaX;
        f32 deltaY;
    };

    struct NYXTY_CORE_API MouseButtonEventData {
        u32 windowID;
        u8 button;
        bool isPressed;
        u8 clicks;
        f32 x;
        f32 y;
    };

    struct NYXTY_CORE_API MouseScrollEventData {
        u32 windowID;
        f32 offsetX;
        f32 offsetY;
        f32 mouseX;
        f32 mouseY;
    };

    struct NYXTY_CORE_API ModuleLoadedEventData {
        u32 moduleID;
        char moduleName[64];  // Fixed-size array, NOT char*
    };

    struct NYXTY_CORE_API ConfigChangedEventData {
        char key[128];       // Key that changed (e.g. "graphics.resolution")
	};

} // namespace Nyxty
