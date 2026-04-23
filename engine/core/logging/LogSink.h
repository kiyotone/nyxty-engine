#pragma once
#include "core/foundation/Core.h"
#include <spdlog/sinks/base_sink.h>
#include <mutex>
#include <vector>
#include <string>

namespace Nyxty {

    struct LogEntry {
        std::string message;
        spdlog::level::level_enum level;
    };

    class NYXTY_CORE_API RingBufferSink : public spdlog::sinks::base_sink<std::mutex> {
    public:
        explicit RingBufferSink(size_t capacity = 512)
            : m_Capacity(capacity) {
        }

        std::vector<LogEntry> CopyEntries() { std::lock_guard lock(mutex_); return m_Entries; }
        void Clear() { std::lock_guard lock(mutex_); m_Entries.clear(); }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            base_sink<std::mutex>::formatter_->format(msg, formatted);

            if (m_Entries.size() >= m_Capacity)
                m_Entries.erase(m_Entries.begin());

            m_Entries.push_back({ fmt::to_string(formatted), msg.level });
        }
        void flush_() override {}

    private:
        size_t m_Capacity;
        std::vector<LogEntry> m_Entries;
    };

} // namespace Nyxty
