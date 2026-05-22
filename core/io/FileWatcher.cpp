#include "core/io/FileWatcher.h"

#include "core/logging/Log.h"

#include <chrono>
#include <filesystem>
#include <unordered_map>

namespace Nyxty {

FileWatcher::~FileWatcher() {
    Stop();
}

void FileWatcher::Start(std::filesystem::path directory, Callback callback) {
    Stop();

    m_Directory = std::move(directory);
    m_Callback = std::move(callback);
    m_Running = true;

    m_Thread = std::thread([this]() {
        std::unordered_map<std::filesystem::path, std::filesystem::file_time_type> seen;
        while (m_Running.load()) {
            std::error_code ec;
            if (std::filesystem::exists(m_Directory, ec)) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(m_Directory, ec)) {
                    if (ec || !entry.is_regular_file(ec)) {
                        continue;
                    }

                    const auto path = entry.path();
                    const auto lastWrite = entry.last_write_time(ec);
                    if (ec) {
                        continue;
                    }

                    auto [it, inserted] = seen.emplace(path, lastWrite);
                    if (!inserted && it->second != lastWrite) {
                        it->second = lastWrite;
                        if (m_Callback) {
                            m_Callback(path);
                        }
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    });
}

void FileWatcher::Stop() {
    m_Running = false;
    if (m_Thread.joinable()) {
        m_Thread.join();
    }
}

} // namespace Nyxty
