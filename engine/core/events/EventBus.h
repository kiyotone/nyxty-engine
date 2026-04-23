#pragma once

#include "core/events/Event.h"
#include "core/foundation/Base.h"
#include "core/logging/Log.h"
#include <cstddef>
#include <entt/signal/delegate.hpp>
#include <mutex>
#include <queue>
#include <vector>
#include "core/foundation/Core.h"

namespace Nyxty {

    // Type-erased callback using EnTT delegate (zero allocation)
    using EventCallback = entt::delegate<void(const Event&)>;

    // ========================================
    // EventBus - Central Event Dispatcher
    // ========================================
    class NYXTY_CORE_API EventBus {
    public:
        EventBus() = default;
        ~EventBus() = default;

        // Prevent copying (singleton pattern)
        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;

        // ========================================
        // Subscribe to Events
        // ========================================
        // Subscribe to an event type
        void Subscribe(EventType type, EventCallback callback, u32 moduleID = 0);

        // Unsubscribe a specific callback
        void Unsubscribe(EventType type, EventCallback callback);

        // Unsubscribe all listeners for a module (used when module unloads)
        void UnsubscribeAll(u32 moduleID);

        // ========================================
        // Dispatch Events (Immediate)
        // ========================================
        // Dispatch event immediately (calls all handlers right now)
        void Dispatch(const Event& event);
        void Queue(const Event& event);
        size_t DispatchQueued(size_t maxEvents = static_cast<size_t>(-1));

        // ========================================
        // Debug/Stats
        // ========================================
        // Get number of listeners for an event type
        size_t GetListenerCount(EventType type) const;
        size_t GetQueuedEventCount() const;
        std::vector<EventType> GetSubscribedEventTypes() const;

        // Get total number of listeners
        size_t GetTotalListeners() const { return m_Listeners.size(); }

    private:
        struct NYXTY_CORE_API Listener {
            EventType type;
            EventCallback callback;
            u32 moduleID;

            // For comparison (to remove specific callbacks)
            bool operator==(const Listener& other) const {
                return type == other.type &&
                    callback == other.callback &&
                    moduleID == other.moduleID;
            }
        };

        std::vector<Listener> m_Listeners;
        mutable std::mutex m_QueueMutex;
        std::queue<Event> m_QueuedEvents;
    };

} // namespace Nyxty
