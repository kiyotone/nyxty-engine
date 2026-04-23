#pragma once
#include "core/foundation/Core.h"
#include "core/foundation/Base.h"
#include <memory>
#include <string>
#include <unordered_map>
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Nyxty {

    // Forward declare so Log.h doesn't need to pull in all of LogSink.h
    class RingBufferSink;

    class NYXTY_CORE_API Log {
    public:
        static void Init();

        static Ref<spdlog::logger>& GetEngineLogger() { return s_EngineLogger; }
        static Ref<spdlog::logger>& GetAppLogger() { return s_AppLogger; }
        static Ref<spdlog::logger>& GetModuleLogger(const std::string& name);

        // Returns the shared ring buffer sink used by the inspector log panel.
        static std::shared_ptr<RingBufferSink> GetSink() { return s_Sink; }

    private:
        static Ref<spdlog::logger> s_EngineLogger;
        static Ref<spdlog::logger> s_AppLogger;
        static std::unordered_map<std::string, Ref<spdlog::logger>> s_ModuleLoggers;
        static std::shared_ptr<RingBufferSink> s_Sink;
    };

} // namespace Nyxty

// ========================================
// Engine Logging Macros
// ========================================
#define NYXTY_ENGINE_TRACE(...)    ::Nyxty::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define NYXTY_ENGINE_INFO(...)     ::Nyxty::Log::GetEngineLogger()->info(__VA_ARGS__)
#define NYXTY_ENGINE_WARN(...)     ::Nyxty::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define NYXTY_ENGINE_ERROR(...)    ::Nyxty::Log::GetEngineLogger()->error(__VA_ARGS__)
#define NYXTY_ENGINE_CRITICAL(...) ::Nyxty::Log::GetEngineLogger()->critical(__VA_ARGS__)
// ========================================
// App Logging Macros
// ========================================
#define NYXTY_APP_TRACE(...)    ::Nyxty::Log::GetAppLogger()->trace(__VA_ARGS__)
#define NYXTY_APP_INFO(...)     ::Nyxty::Log::GetAppLogger()->info(__VA_ARGS__)
#define NYXTY_APP_WARN(...)     ::Nyxty::Log::GetAppLogger()->warn(__VA_ARGS__)
#define NYXTY_APP_ERROR(...)    ::Nyxty::Log::GetAppLogger()->error(__VA_ARGS__)
#define NYXTY_APP_CRITICAL(...) ::Nyxty::Log::GetAppLogger()->critical(__VA_ARGS__)
// ========================================
// Module Logging Macros
// ========================================
#define NYXTY_MODULE_TRACE(name, ...)    ::Nyxty::Log::GetModuleLogger(name)->trace(__VA_ARGS__)
#define NYXTY_MODULE_INFO(name, ...)     ::Nyxty::Log::GetModuleLogger(name)->info(__VA_ARGS__)
#define NYXTY_MODULE_WARN(name, ...)     ::Nyxty::Log::GetModuleLogger(name)->warn(__VA_ARGS__)
#define NYXTY_MODULE_ERROR(name, ...)    ::Nyxty::Log::GetModuleLogger(name)->error(__VA_ARGS__)
#define NYXTY_MODULE_CRITICAL(name, ...) ::Nyxty::Log::GetModuleLogger(name)->critical(__VA_ARGS__)
