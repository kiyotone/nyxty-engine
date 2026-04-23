#pragma once
#include "core/foundation/Core.h"
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <cstdint>

namespace Nyxty {

    class NYXTY_CORE_API Renderer {
    public:
        struct Desc {
            void* nativeWindowHandle{ nullptr };
            void* nativeDisplayHandle{ nullptr };
            uint16_t width{ 0 };
            uint16_t height{ 0 };
            bgfx::RendererType::Enum type = bgfx::RendererType::Count;
        };

        bool Init(const Desc& desc);
        void Shutdown();
        bool Resize(uint16_t width, uint16_t height);
        void BeginFrame(bgfx::ViewId view = 0);
        void EndFrame();

        uint16_t GetWidth() const { return m_Width; }
        uint16_t GetHeight() const { return m_Height; }

    private:
        uint16_t m_Width{ 0 };
        uint16_t m_Height{ 0 };
        uint32_t m_ResetFlags{ BGFX_RESET_VSYNC };
        bool     m_Initialized{ false };
    };

} // namespace Nyxty