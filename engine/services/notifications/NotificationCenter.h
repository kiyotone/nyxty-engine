#pragma once

#include "core/foundation/Base.h"
#include "core/foundation/Core.h"

#include <mutex>
#include <string>
#include <vector>

namespace Nyxty {

    enum class NotificationSeverity {
        Info,
        Success,
        Warning,
        Error
    };

    struct Notification {
        u64 id{ 0 };
        NotificationSeverity severity{ NotificationSeverity::Info };
        std::string title;
        std::string message;
        double lifetimeSeconds{ 5.0 };
    };

    class NYXTY_CORE_API NotificationCenter {
    public:
        u64 Push(NotificationSeverity severity, std::string title, std::string message, double lifetimeSeconds = 5.0);
        std::vector<Notification> Copy() const;
        void Dismiss(u64 id);
        void Clear();

    private:
        mutable std::mutex m_Mutex;
        std::vector<Notification> m_Notifications;
        u64 m_NextID{ 1 };
    };

} // namespace Nyxty
