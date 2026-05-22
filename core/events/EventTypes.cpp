#include "core/events/EventTypes.h"

namespace Nyxty {

    const char* EventTypeToString(EventType type) {
        switch (type) {
            // System
        case EVENT_NONE:              return "NONE";
        case EVENT_APP_INIT:          return "APP_INIT";
        case EVENT_APP_SHUTDOWN:      return "APP_SHUTDOWN";
        case EVENT_APP_UPDATE:        return "APP_UPDATE";
        case EVENT_APP_RENDER:        return "APP_RENDER";
        case EVENT_MODULE_LOADED:     return "MODULE_LOADED";
        case EVENT_MODULE_UNLOADED:   return "MODULE_UNLOADED";
        case EVENT_MODULE_RELOAD:     return "MODULE_RELOAD";
		case EVENT_CONFIG_CHANGED:    return "CONFIG_CHANGED";

            // Input - Keyboard
        case EVENT_KEY_PRESS:         return "KEY_PRESS";
        case EVENT_KEY_RELEASE:       return "KEY_RELEASE";
        case EVENT_KEY_REPEAT:        return "KEY_REPEAT";

            // Input - Mouse
        case EVENT_MOUSE_MOVE:              return "MOUSE_MOVE";
        case EVENT_MOUSE_BUTTON_PRESS:      return "MOUSE_BUTTON_PRESS";
        case EVENT_MOUSE_BUTTON_RELEASE:    return "MOUSE_BUTTON_RELEASE";
        case EVENT_MOUSE_SCROLL:            return "MOUSE_SCROLL";

            // Input - Gamepad
        case EVENT_GAMEPAD_CONNECTED:       return "GAMEPAD_CONNECTED";
        case EVENT_GAMEPAD_DISCONNECTED:    return "GAMEPAD_DISCONNECTED";
        case EVENT_GAMEPAD_BUTTON:          return "GAMEPAD_BUTTON";
        case EVENT_GAMEPAD_AXIS:            return "GAMEPAD_AXIS";

            // Window
        case EVENT_WINDOW_RESIZE:       return "WINDOW_RESIZE";
        case EVENT_WINDOW_CLOSE:        return "WINDOW_CLOSE";
        case EVENT_WINDOW_FOCUS:        return "WINDOW_FOCUS";
        case EVENT_WINDOW_LOST_FOCUS:   return "WINDOW_LOST_FOCUS";
        case EVENT_WINDOW_MOVED:        return "WINDOW_MOVED";
        case EVENT_WINDOW_MINIMIZED:    return "WINDOW_MINIMIZED";
        case EVENT_WINDOW_MAXIMIZED:    return "WINDOW_MAXIMIZED";
        case EVENT_WINDOW_RESTORED:     return "WINDOW_RESTORED";

            // Renderer
        case EVENT_RENDER_FRAME_BEGIN:      return "RENDER_FRAME_BEGIN";
        case EVENT_RENDER_FRAME_END:        return "RENDER_FRAME_END";
        case EVENT_RENDER_VIEWPORT_RESIZE:  return "RENDER_VIEWPORT_RESIZE";

            // Assets
        case EVENT_ASSET_LOADED:    return "ASSET_LOADED";
        case EVENT_ASSET_UNLOADED:  return "ASSET_UNLOADED";
        case EVENT_ASSET_RELOADED:  return "ASSET_RELOADED";

        default:
            if (type >= EVENT_CUSTOM_START) {
                return "CUSTOM_EVENT";
            }
            return "UNKNOWN";
        }
    }

    EventCategory GetEventCategory(EventType type) {
        // System events
        if (type >= EVENT_APP_INIT && type < EVENT_KEY_PRESS) {
            return EVENT_CATEGORY_SYSTEM;
        }

        // Keyboard events
        if (type >= EVENT_KEY_PRESS && type <= EVENT_KEY_REPEAT) {
            return static_cast<EventCategory>(EVENT_CATEGORY_INPUT | EVENT_CATEGORY_KEYBOARD);
        }

        // Mouse events
        if (type >= EVENT_MOUSE_MOVE && type <= EVENT_MOUSE_SCROLL) {
            return static_cast<EventCategory>(EVENT_CATEGORY_INPUT | EVENT_CATEGORY_MOUSE);
        }

        // Gamepad events
        if (type >= EVENT_GAMEPAD_CONNECTED && type <= EVENT_GAMEPAD_AXIS) {
            return static_cast<EventCategory>(EVENT_CATEGORY_INPUT);
        }

        // Window events
        if (type >= EVENT_WINDOW_RESIZE && type < EVENT_RENDER_FRAME_BEGIN) {
            return EVENT_CATEGORY_WINDOW;
        }

        // Renderer events
        if (type >= EVENT_RENDER_FRAME_BEGIN && type < EVENT_ASSET_LOADED) {
            return EVENT_CATEGORY_RENDERER;
        }

        // Asset events
        if (type >= EVENT_ASSET_LOADED && type < EVENT_CUSTOM_START) {
            return EVENT_CATEGORY_ASSET;
        }

        // Custom events
        if (type >= EVENT_CUSTOM_START) {
            return EVENT_CATEGORY_CUSTOM;
        }

        return EVENT_CATEGORY_NONE;
    }

} // namespace Nyxty