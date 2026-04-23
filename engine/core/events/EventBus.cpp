#include "core/events/EventBus.h"
#include "core/events/EventTypes.h"
#include <algorithm>
#include "core/foundation/Debug.h"


namespace Nyxty {

    void EventBus::Subscribe(EventType type, EventCallback callback, u32 moduleID) {
        NYXTY_CORE_ASSERT(callback, "Cannot subscribe with null callback!");

        // Check for duplicate subscription
        for (const auto& listener : m_Listeners) {
            if (listener.type == type && listener.callback == callback && listener.moduleID == moduleID) {
                NYXTY_ENGINE_WARN("Duplicate subscription detected for event type: {}", EventTypeToString(type));
                return;
            }
        }

        // Add new listener
        m_Listeners.push_back({ type, callback, moduleID });

        NYXTY_ENGINE_TRACE("Subscribed to event: {} (Module ID: {}, Total listeners: {})",
            EventTypeToString(type), moduleID, m_Listeners.size());
    }

    void EventBus::Unsubscribe(EventType type, EventCallback callback) {
        auto it = std::remove_if(m_Listeners.begin(), m_Listeners.end(),
            [type, callback](const Listener& listener) {
                return listener.type == type && listener.callback == callback;
            });

        if (it != m_Listeners.end()) {
            size_t removed = std::distance(it, m_Listeners.end());
            m_Listeners.erase(it, m_Listeners.end());
            NYXTY_ENGINE_TRACE("Unsubscribed from event: {} ({} listeners removed)",
                EventTypeToString(type), removed);
        }
    }

    void EventBus::UnsubscribeAll(u32 moduleID) {
        auto it = std::remove_if(m_Listeners.begin(), m_Listeners.end(),
            [moduleID](const Listener& listener) {
                return listener.moduleID == moduleID;
            });

        if (it != m_Listeners.end()) {
            size_t removed = std::distance(it, m_Listeners.end());
            m_Listeners.erase(it, m_Listeners.end());
            NYXTY_ENGINE_INFO("Unsubscribed all listeners for module ID: {} ({} listeners removed)",
                moduleID, removed);
        }
    }

    void EventBus::Dispatch(const Event& event) {
        NYXTY_CORE_ASSERT(event.type != EVENT_NONE, "Cannot dispatch EVENT_NONE!");

        // Count how many listeners will receive this event
        size_t listenerCount = 0;
        for (const auto& listener : m_Listeners) {
            if (listener.type == event.type) {
                listenerCount++;
            }
        }

        if (listenerCount == 0) {
            NYXTY_ENGINE_TRACE("No listeners for event: {}", EventTypeToString(event.type));
            return;
        }

        NYXTY_ENGINE_TRACE("Dispatching event: {} to {} listener(s)",
            EventTypeToString(event.type), listenerCount);

        // Dispatch to all matching listeners
        for (const auto& listener : m_Listeners) {
            if (listener.type == event.type) {
                listener.callback(event);
            }
        }
    }

    void EventBus::Queue(const Event& event) {
        if (event.type == EVENT_NONE) {
            return;
        }

        std::scoped_lock lock(m_QueueMutex);
        m_QueuedEvents.push(event);
    }

    size_t EventBus::DispatchQueued(size_t maxEvents) {
        size_t dispatched = 0;

        while (dispatched < maxEvents) {
            Event event;
            {
                std::scoped_lock lock(m_QueueMutex);
                if (m_QueuedEvents.empty()) {
                    break;
                }

                event = m_QueuedEvents.front();
                m_QueuedEvents.pop();
            }

            Dispatch(event);
            ++dispatched;
        }

        return dispatched;
    }

    size_t EventBus::GetListenerCount(EventType type) const {
        return std::count_if(m_Listeners.begin(), m_Listeners.end(),
            [type](const Listener& listener) {
                return listener.type == type;
            });
    }

    size_t EventBus::GetQueuedEventCount() const {
        std::scoped_lock lock(m_QueueMutex);
        return m_QueuedEvents.size();
    }

    std::vector<EventType> EventBus::GetSubscribedEventTypes() const {
        std::vector<EventType> types;
        for (const Listener& listener : m_Listeners) {
            if (std::find(types.begin(), types.end(), listener.type) == types.end()) {
                types.push_back(listener.type);
            }
        }

        std::sort(types.begin(), types.end());
        return types;
    }

} // namespace Nyxty
