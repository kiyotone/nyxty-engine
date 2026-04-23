#include "services/window/Window.h"
#include "core/events/Event.h"
#include "core/events/EventBusInstance.h"
#include "core/logging/Log.h"

#include <SDL3/SDL.h>
#include <atomic>
#include <unordered_map>
#include <vector>

namespace Nyxty {

    namespace {

        std::unordered_map<SDL_WindowID, Window*> s_Windows;
        std::unordered_map<Window::EventHookHandle, std::pair<Window::EventHook, void*>> s_EventHooks;
        std::atomic<Window::EventHookHandle> s_NextEventHookHandle{ 1 };
        Window::EventHookHandle s_LegacyEventHookHandle = 0;

        template<typename T>
        void DispatchEvent(EventType type, const T& payload) {
            EventBusInstance::Get()->Dispatch(Event::Create(type, payload));
        }

    } // namespace
    bool Window::InitSDL() {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
            NYXTY_ENGINE_ERROR("SDL_Init failed: {}", SDL_GetError());
            return false;
        }
        NYXTY_ENGINE_INFO("SDL3 initialized.");
        return true;
    }

    void Window::ShutdownSDL() {
        SDL_Quit();
        NYXTY_ENGINE_INFO("SDL3 shut down.");
    }

    void Window::PollEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (!s_EventHooks.empty()) {
                std::vector<std::pair<EventHook, void*>> hooks;
                hooks.reserve(s_EventHooks.size());
                for (const auto& [_, entry] : s_EventHooks) {
                    if (entry.first) {
                        hooks.push_back(entry);
                    }
                }

                for (const auto& [hook, userData] : hooks) {
                    hook(e, userData);
                }
            }

            Window* trackedWindow = nullptr;
            SDL_Window* rawWindow = nullptr;
            SDL_WindowID windowID = 0;

            if (e.type >= SDL_EVENT_WINDOW_FIRST && e.type <= SDL_EVENT_WINDOW_LAST) {
                windowID = e.window.windowID;
                auto it = s_Windows.find(windowID);
                if (it != s_Windows.end()) {
                    trackedWindow = it->second;
                }
                rawWindow = SDL_GetWindowFromID(windowID);
            }

            switch (e.type) {
            case SDL_EVENT_QUIT:
                if (s_Windows.empty()) {
                    EventBusInstance::Get()->Dispatch(Event::Create<u32>(EVENT_APP_SHUTDOWN, 0u));
                    break;
                }

                for (const auto& [trackedWindowID, tracked] : s_Windows) {
                    if (!tracked) {
                        continue;
                    }

                    tracked->m_ShouldClose = true;
                    DispatchEvent(EVENT_WINDOW_CLOSE, WindowEventData{ static_cast<u32>(trackedWindowID) });
                }
                break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (trackedWindow) {
                    trackedWindow->m_ShouldClose = true;
                }
                DispatchEvent(EVENT_WINDOW_CLOSE, WindowEventData{ static_cast<u32>(windowID) });
                break;

            case SDL_EVENT_WINDOW_MOVED:
                DispatchEvent(EVENT_WINDOW_MOVED, WindowMoveEventData{
                    static_cast<u32>(windowID),
                    e.window.data1,
                    e.window.data2
                });
                break;

            case SDL_EVENT_WINDOW_RESIZED: {
                int pixelWidth = e.window.data1;
                int pixelHeight = e.window.data2;
                float dpiScale = 1.0f;
                if (rawWindow) {
                    SDL_GetWindowSizeInPixels(rawWindow, &pixelWidth, &pixelHeight);
                    dpiScale = SDL_GetWindowDisplayScale(rawWindow);
                }
                if (trackedWindow) {
                    trackedWindow->m_Width = pixelWidth;
                    trackedWindow->m_Height = pixelHeight;
                }
                DispatchEvent(EVENT_WINDOW_RESIZE, WindowResizeEventData{
                    static_cast<u32>(windowID),
                    pixelWidth,
                    pixelHeight,
                    dpiScale
                });
                break;
            }

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                DispatchEvent(EVENT_WINDOW_FOCUS, WindowEventData{ static_cast<u32>(windowID) });
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                DispatchEvent(EVENT_WINDOW_LOST_FOCUS, WindowEventData{ static_cast<u32>(windowID) });
                break;

            case SDL_EVENT_WINDOW_MINIMIZED:
                DispatchEvent(EVENT_WINDOW_MINIMIZED, WindowEventData{ static_cast<u32>(windowID) });
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
                DispatchEvent(EVENT_WINDOW_MAXIMIZED, WindowEventData{ static_cast<u32>(windowID) });
                break;

            case SDL_EVENT_WINDOW_RESTORED:
                DispatchEvent(EVENT_WINDOW_RESTORED, WindowEventData{ static_cast<u32>(windowID) });
                break;

            case SDL_EVENT_KEY_DOWN: {
                const EventType type = e.key.repeat ? EVENT_KEY_REPEAT : EVENT_KEY_PRESS;
                DispatchEvent(type, KeyPressEventData{
                    static_cast<u32>(e.key.windowID),
                    static_cast<i32>(e.key.key),
                    static_cast<u32>(e.key.mod),
                    true,
                    e.key.repeat
                });
                break;
            }

            case SDL_EVENT_KEY_UP:
                DispatchEvent(EVENT_KEY_RELEASE, KeyPressEventData{
                    static_cast<u32>(e.key.windowID),
                    static_cast<i32>(e.key.key),
                    static_cast<u32>(e.key.mod),
                    false,
                    false
                });
                break;

            case SDL_EVENT_MOUSE_MOTION:
                DispatchEvent(EVENT_MOUSE_MOVE, MouseMoveEventData{
                    static_cast<u32>(e.motion.windowID),
                    e.motion.x,
                    e.motion.y,
                    e.motion.xrel,
                    e.motion.yrel
                });
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                DispatchEvent(EVENT_MOUSE_BUTTON_PRESS, MouseButtonEventData{
                    static_cast<u32>(e.button.windowID),
                    e.button.button,
                    true,
                    e.button.clicks,
                    e.button.x,
                    e.button.y
                });
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                DispatchEvent(EVENT_MOUSE_BUTTON_RELEASE, MouseButtonEventData{
                    static_cast<u32>(e.button.windowID),
                    e.button.button,
                    false,
                    e.button.clicks,
                    e.button.x,
                    e.button.y
                });
                break;

            case SDL_EVENT_MOUSE_WHEEL: {
                float scrollX = e.wheel.x;
                float scrollY = e.wheel.y;
                if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
                    scrollX *= -1.0f;
                    scrollY *= -1.0f;
                }
                DispatchEvent(EVENT_MOUSE_SCROLL, MouseScrollEventData{
                    static_cast<u32>(e.wheel.windowID),
                    scrollX,
                    scrollY,
                    e.wheel.mouse_x,
                    e.wheel.mouse_y
                });
                break;
            }

            default:
                break;
            }
        }
    }

    Window::EventHookHandle Window::AddEventHook(EventHook hook, void* userData) {
        if (!hook) {
            return 0;
        }

        const EventHookHandle handle = s_NextEventHookHandle.fetch_add(1, std::memory_order_relaxed);
        s_EventHooks[handle] = std::make_pair(hook, userData);
        return handle;
    }

    void Window::RemoveEventHook(EventHookHandle handle) {
        if (handle == 0) {
            return;
        }

        s_EventHooks.erase(handle);
        if (s_LegacyEventHookHandle == handle) {
            s_LegacyEventHookHandle = 0;
        }
    }

    void Window::SetEventHook(EventHook hook, void* userData) {
        ClearEventHook();
        if (hook) {
            s_LegacyEventHookHandle = AddEventHook(hook, userData);
        }
    }

    void Window::ClearEventHook() {
        if (s_LegacyEventHookHandle != 0) {
            s_EventHooks.erase(s_LegacyEventHookHandle);
            s_LegacyEventHookHandle = 0;
        }
    }

    std::vector<WindowSnapshot> Window::GetOpenWindowSnapshots() {
        std::vector<WindowSnapshot> snapshots;
        snapshots.reserve(s_Windows.size());

        for (const auto& [windowID, window] : s_Windows) {
            if (!window || !window->m_Handle) {
                continue;
            }

            int posX = 0;
            int posY = 0;
            SDL_GetWindowPosition(window->m_Handle, &posX, &posY);

            int pixelWidth = window->m_Width;
            int pixelHeight = window->m_Height;
            SDL_GetWindowSizeInPixels(window->m_Handle, &pixelWidth, &pixelHeight);

            const SDL_WindowFlags flags = SDL_GetWindowFlags(window->m_Handle);

            WindowSnapshot snapshot{};
            snapshot.windowID = static_cast<u32>(windowID);
            snapshot.title = SDL_GetWindowTitle(window->m_Handle);
            snapshot.x = posX;
            snapshot.y = posY;
            snapshot.width = pixelWidth;
            snapshot.height = pixelHeight;
            snapshot.visible = (flags & SDL_WINDOW_HIDDEN) == 0;
            snapshot.focused = (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
            snapshot.shouldClose = window->m_ShouldClose;
            snapshots.push_back(std::move(snapshot));
        }

        std::sort(snapshots.begin(), snapshots.end(), [](const WindowSnapshot& lhs, const WindowSnapshot& rhs) {
            return lhs.windowID < rhs.windowID;
        });
        return snapshots;
    }

    bool Window::Init(const Desc& desc) {
        SDL_WindowFlags flags = SDL_WINDOW_HIGH_PIXEL_DENSITY;

        if (!desc.decorated) {
            flags |= SDL_WINDOW_BORDERLESS;
        }
        if (desc.transparent) {
            flags |= SDL_WINDOW_TRANSPARENT;
        }
        if (desc.floating) {
            flags |= SDL_WINDOW_ALWAYS_ON_TOP;
        }
        if (desc.resizable) {
            flags |= SDL_WINDOW_RESIZABLE;
        }

        m_Handle = SDL_CreateWindow(desc.title.c_str(), desc.width, desc.height, flags);
        if (!m_Handle) {
            NYXTY_ENGINE_ERROR("SDL_CreateWindow failed: {}", SDL_GetError());
            return false;
        }

        m_WindowID = SDL_GetWindowID(m_Handle);
        m_ShouldClose = false;
        if (!SDL_GetWindowSizeInPixels(m_Handle, &m_Width, &m_Height)) {
            m_Width = desc.width;
            m_Height = desc.height;
        }

        s_Windows[m_WindowID] = this;

        NYXTY_ENGINE_INFO("Window created: '{}' ({}x{})", desc.title, m_Width, m_Height);
        return true;
    }

    void Window::Shutdown() {
        if (m_Handle) {
            s_Windows.erase(m_WindowID);
            SDL_DestroyWindow(m_Handle);
            m_Handle = nullptr;
            m_WindowID = 0;
        }
    }

    bool Window::ShouldClose() const {
        return m_ShouldClose;
    }

    void Window::ClearCloseRequest() {
        m_ShouldClose = false;
    }

    void* Window::GetNativeHandle() const {
#if defined(_WIN32)
        return (void*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(m_Handle),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__)
        return (void*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(m_Handle),
            SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#else
        return (void*)(uintptr_t)SDL_GetNumberProperty(
            SDL_GetWindowProperties(m_Handle),
            SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
#endif
    }

} // namespace Nyxty
