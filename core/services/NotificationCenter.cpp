#include "core/services/NotificationCenter.h"

#include <algorithm>
#include <utility>

namespace Nyxty {

u64 NotificationCenter::Push(
    NotificationSeverity severity,
    std::string title,
    std::string message,
    double lifetimeSeconds) {
    std::scoped_lock lock(m_Mutex);

    const u64 id = m_NextID++;
    m_Notifications.push_back(Notification{
        id,
        severity,
        std::move(title),
        std::move(message),
        lifetimeSeconds
    });
    return id;
}

std::vector<Notification> NotificationCenter::Copy() const {
    std::scoped_lock lock(m_Mutex);
    return m_Notifications;
}

void NotificationCenter::Dismiss(u64 id) {
    std::scoped_lock lock(m_Mutex);
    m_Notifications.erase(
        std::remove_if(m_Notifications.begin(), m_Notifications.end(), [id](const Notification& notification) {
            return notification.id == id;
        }),
        m_Notifications.end());
}

void NotificationCenter::Clear() {
    std::scoped_lock lock(m_Mutex);
    m_Notifications.clear();
}

} // namespace Nyxty
