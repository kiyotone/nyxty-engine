#pragma once

#include "core/foundation/Base.h"
#include "core/foundation/Core.h"

#include <atomic>
#include <filesystem>
#include <functional>
#include <thread>

namespace Nyxty {

    class NYXTY_CORE_API FileWatcher {
    public:
        using Callback = std::function<void(const std::filesystem::path&)>;

        FileWatcher() = default;
        ~FileWatcher();

        void Start(std::filesystem::path directory, Callback callback);
        void Stop();
        bool IsRunning() const { return m_Running.load(); }

    private:
        std::filesystem::path m_Directory;
        Callback m_Callback;
        std::thread m_Thread;
        std::atomic_bool m_Running{ false };
    };

} // namespace Nyxty
