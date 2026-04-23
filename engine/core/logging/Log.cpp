#include "core/logging/Log.h"
#include "core/logging/LogSink.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Nyxty {

    // Static member definitions
    Ref<spdlog::logger>                    Log::s_EngineLogger;
    Ref<spdlog::logger>                    Log::s_AppLogger;
    std::unordered_map<std::string, Ref<spdlog::logger>> Log::s_ModuleLoggers;
    std::shared_ptr<RingBufferSink>        Log::s_Sink;

    void Log::Init() {
        // Create shared ring buffer sink first
        s_Sink = std::make_shared<RingBufferSink>(512);
        s_Sink->set_pattern("[%T] [%n] [%^%l%$] %v");

        // Engine logger
        s_EngineLogger = spdlog::stdout_color_mt("NYXTY_ENGINE");
        s_EngineLogger->set_level(spdlog::level::trace);
        s_EngineLogger->sinks().push_back(s_Sink);

        // App logger
        s_AppLogger = spdlog::stdout_color_mt("NYXTY_APP");
        s_AppLogger->set_level(spdlog::level::trace);
        s_AppLogger->sinks().push_back(s_Sink);
    }

    Ref<spdlog::logger>& Log::GetModuleLogger(const std::string& name) {
        auto it = s_ModuleLoggers.find(name);
        if (it != s_ModuleLoggers.end())
            return it->second;

        auto logger = spdlog::stdout_color_mt("NYXTY_" + name);
        logger->set_level(spdlog::level::trace);
        logger->sinks().push_back(s_Sink);
        s_ModuleLoggers[name] = logger;
        return s_ModuleLoggers[name];
    }

} // namespace Nyxty
