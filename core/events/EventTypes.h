#pragma once

#include "core/foundation/Base.h"

namespace Nyxty {

    // ========================================
    // Event Type Registry
    // ========================================
    // Central enum for all event types
    // Rules:
    // - System events: 1-99
    // - Input events: 100-199
    // - Window events: 200-299
    // - Renderer events: 300-399
    // - Asset events: 400-499
    // - Custom module events: 1000+

    enum EventType : u32 {
        EVENT_NONE = 0,

        // ========================================
        // System Events (1-99)
        // ========================================
        EVENT_APP_INIT = 1,
        EVENT_APP_SHUTDOWN = 2,
        EVENT_APP_UPDATE = 3,
        EVENT_APP_RENDER = 4,

        EVENT_MODULE_LOADED = 10,
        EVENT_MODULE_UNLOADED = 11,
        EVENT_MODULE_RELOAD = 12,

		EVENT_CONFIG_CHANGED = 20,

        // ========================================
        // Input Events (100-199)
        // ========================================
        EVENT_KEY_PRESS = 100,
        EVENT_KEY_RELEASE = 101,
        EVENT_KEY_REPEAT = 102,

        EVENT_MOUSE_MOVE = 110,
        EVENT_MOUSE_BUTTON_PRESS = 111,
        EVENT_MOUSE_BUTTON_RELEASE = 112,
        EVENT_MOUSE_SCROLL = 113,

        EVENT_GAMEPAD_CONNECTED = 120,
        EVENT_GAMEPAD_DISCONNECTED = 121,
        EVENT_GAMEPAD_BUTTON = 122,
        EVENT_GAMEPAD_AXIS = 123,

        // ========================================
        // Window Events (200-299)
        // ========================================
        EVENT_WINDOW_RESIZE = 200,
        EVENT_WINDOW_CLOSE = 201,
        EVENT_WINDOW_FOCUS = 202,
        EVENT_WINDOW_LOST_FOCUS = 203,
        EVENT_WINDOW_MOVED = 204,
        EVENT_WINDOW_MINIMIZED = 205,
        EVENT_WINDOW_MAXIMIZED = 206,
        EVENT_WINDOW_RESTORED = 207,

        // ========================================
        // Renderer Events (300-399)
        // ========================================
        EVENT_RENDER_FRAME_BEGIN = 300,
        EVENT_RENDER_FRAME_END = 301,
        EVENT_RENDER_VIEWPORT_RESIZE = 302,

        // ========================================
        // Asset Events (400-499)
        // ========================================
        EVENT_ASSET_LOADED = 400,
        EVENT_ASSET_UNLOADED = 401,
        EVENT_ASSET_RELOADED = 402,

        // ========================================
        // Custom Module Events (1000+)
        // ========================================
        EVENT_CUSTOM_START = 1000

        // Modules can define their own events starting from EVENT_CUSTOM_START
        // Example: EVENT_PHYSICS_COLLISION = EVENT_CUSTOM_START + 1
    };

    // ========================================
    // Event Category Flags (for filtering)
    // ========================================
    enum EventCategory : u32 {
        EVENT_CATEGORY_NONE = 0,
        EVENT_CATEGORY_SYSTEM = BIT(0),
        EVENT_CATEGORY_INPUT = BIT(1),
        EVENT_CATEGORY_KEYBOARD = BIT(2),
        EVENT_CATEGORY_MOUSE = BIT(3),
        EVENT_CATEGORY_WINDOW = BIT(4),
        EVENT_CATEGORY_RENDERER = BIT(5),
        EVENT_CATEGORY_ASSET = BIT(6),
        EVENT_CATEGORY_CUSTOM = BIT(7)
    };

    // ========================================
    // Helper Functions
    // ========================================

    // Convert event type to string (for debugging/logging)
    NYXTY_CORE_API const char* EventTypeToString(EventType type);

    // Get category for an event type
    NYXTY_CORE_API EventCategory GetEventCategory(EventType type);

    // Check if event belongs to a category
    inline bool IsEventInCategory(EventType type, EventCategory category) {
        return (GetEventCategory(type) & category) != 0;
    }

} // namespace Nyxty
