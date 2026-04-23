#include "rendering/backend/Renderer.h"
#include "core/logging/Log.h"

#include <array>

namespace Nyxty {

    bool Renderer::Init(const Desc& desc) {
        if (!desc.nativeWindowHandle) {
            NYXTY_ENGINE_ERROR("Renderer initialization failed: native window handle is null.");
            return false;
        }

        bgfx::PlatformData platformData{};
        platformData.ndt = desc.nativeDisplayHandle;
        platformData.nwh = desc.nativeWindowHandle;

        auto tryInit = [&](bgfx::RendererType::Enum type) {
            bgfx::Init init{};
            init.type = type;
            init.platformData = platformData;
            init.resolution.width = desc.width;
            init.resolution.height = desc.height;
            init.resolution.reset = BGFX_RESET_VSYNC;
            init.fallback = true;
#if defined(BX_CONFIG_DEBUG) && BX_CONFIG_DEBUG
            init.debug = true;
#endif
            NYXTY_ENGINE_INFO(
                "Attempting bgfx backend '{}' on native window {}.",
                bgfx::getRendererName(type),
                fmt::ptr(desc.nativeWindowHandle));
            return bgfx::init(init);
        };

        std::array<bgfx::RendererType::Enum, 5> candidates{
            desc.type,
#if defined(_WIN32)
            bgfx::RendererType::Direct3D12,
            bgfx::RendererType::Direct3D11,
            bgfx::RendererType::Vulkan,
            bgfx::RendererType::OpenGL
#else
            bgfx::RendererType::Count,
            bgfx::RendererType::Vulkan,
            bgfx::RendererType::OpenGL,
            bgfx::RendererType::Count
#endif
        };

        auto alreadyAttempted = [&](size_t uptoIndex, bgfx::RendererType::Enum type) {
            for (size_t i = 0; i < uptoIndex; ++i) {
                if (candidates[i] == type) {
                    return true;
                }
            }
            return false;
        };

        bool initialized = false;
        for (size_t i = 0; i < candidates.size(); ++i) {
            const bgfx::RendererType::Enum candidate = candidates[i];
            if (candidate == bgfx::RendererType::Noop || alreadyAttempted(i, candidate)) {
                continue;
            }

            if (tryInit(candidate)) {
                initialized = true;
                break;
            }

            NYXTY_ENGINE_WARN("bgfx backend '{}' failed to initialize.", bgfx::getRendererName(candidate));
        }

        if (!initialized && desc.type != bgfx::RendererType::Count) {
            NYXTY_ENGINE_INFO("Attempting bgfx automatic backend selection after explicit backend failures.");
            if (tryInit(bgfx::RendererType::Count)) {
                initialized = true;
            }
        }

        if (!initialized) {
            NYXTY_ENGINE_ERROR("bgfx::init failed for all attempted backends.");
            return false;
        }

        m_Width = desc.width;
        m_Height = desc.height;
        m_ResetFlags = BGFX_RESET_VSYNC;
        m_Initialized = true;

        NYXTY_ENGINE_INFO("Renderer initialized ({}x{}) backend={}",
            m_Width, m_Height,
            bgfx::getRendererName(bgfx::getRendererType()));
        return true;
    }

    void Renderer::Shutdown() {
        if (m_Initialized) {
            bgfx::shutdown();
            m_Initialized = false;
        }
    }

    bool Renderer::Resize(uint16_t width, uint16_t height) {
        if (!m_Initialized) {
            return false;
        }

        m_Width = width > 0 ? width : 1;
        m_Height = height > 0 ? height : 1;
        bgfx::reset(m_Width, m_Height, m_ResetFlags);
        NYXTY_ENGINE_INFO("Renderer resized to {}x{}", m_Width, m_Height);
        return true;
    }

    void Renderer::BeginFrame(bgfx::ViewId view) {
        bgfx::setViewClear(view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x1f2430ff, 1.0f, 0);
        bgfx::setViewRect(view, 0, 0, m_Width, m_Height);
        bgfx::touch(view);
    }

    void Renderer::EndFrame() {
        bgfx::frame();
    }

} // namespace Nyxty
