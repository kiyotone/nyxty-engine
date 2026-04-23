#pragma once
#include "core/foundation/Base.h"
#include "core/foundation/Core.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>

namespace Nyxty {

    struct NYXTY_CORE_API WindowSnapshot {
        u32 windowID{ 0 };
        std::string title;
        int x{ 0 };
        int y{ 0 };
        int width{ 0 };
        int height{ 0 };
        bool visible{ false };
        bool focused{ false };
        bool shouldClose{ false };
    };

    class NYXTY_CORE_API Window {
    public:
        using EventHook = void(*)(const SDL_Event&, void*);
        using EventHookHandle = u64;

        struct Desc {
            std::string title = "Nyxty";
            int         width = 1280;
            int         height = 720;
            bool        decorated = true;
            bool        floating = false;
            bool        transparent = false;
            bool        resizable = false;
        };

        bool         Init(const Desc& desc);
        void         Shutdown();
        bool         ShouldClose() const;
        void         ClearCloseRequest();

        SDL_Window*   GetHandle() const { return m_Handle; }
        SDL_WindowID  GetID() const { return m_WindowID; }
        void*         GetNativeHandle() const;
        int           GetWidth() const { return m_Width; }
        int           GetHeight() const { return m_Height; }

        static bool   InitSDL();
        static void   ShutdownSDL();
        static void   PollEvents();
        static EventHookHandle AddEventHook(EventHook hook, void* userData);
        static void   RemoveEventHook(EventHookHandle handle);
        static void   SetEventHook(EventHook hook, void* userData);
        static void   ClearEventHook();
        static std::vector<WindowSnapshot> GetOpenWindowSnapshots();

    private:
        SDL_Window*   m_Handle{ nullptr };
        SDL_WindowID  m_WindowID{ 0 };
        int           m_Width{ 0 };
        int           m_Height{ 0 };
        bool          m_ShouldClose{ false };
    };

} // namespace Nyxty
