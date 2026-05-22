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
        explicit RingBufferSink(size_t capacity)
            : m_Buffer(capacity)
            , m_Capacity(capacity)
            , m_Head(0)
            , m_Size(0)
        {
        }

        std::vector<LogEntry> CopyEntries() {
            std::lock_guard lock(mutex_);

            std::vector<LogEntry> result;
            result.reserve(m_Size);

            size_t index = (m_Head + m_Capacity - m_Size) % m_Capacity;

            for (size_t i = 0; i < m_Size; i++) {
                result.push_back(m_Buffer[index]);
                index = (index + 1) % m_Capacity;
            }

            return result;
        }
        void Clear() { std::lock_guard lock(mutex_); m_Head = 0; m_Size = 0; }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t formatted;
            base_sink<std::mutex>::formatter_->format(msg, formatted);

            m_Buffer[m_Head] = {
                fmt::to_string(formatted),
                msg.level
            };

            m_Head = (m_Head + 1) % m_Capacity;

            if (m_Size < m_Capacity)
                m_Size++;
        }
        void flush_() override {}

    private:
        std::vector<LogEntry> m_Buffer;
        size_t m_Capacity;
        size_t m_Head{ 0 };
        size_t m_Size{ 0 };
    };

} // namespace Nyxty
