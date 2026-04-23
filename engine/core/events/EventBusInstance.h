#pragma once

#include "core/events/EventBus.h"
#include "core/foundation/Core.h"

namespace Nyxty {

    class NYXTY_CORE_API EventBusInstance {
    public:
        static EventBus* Get() {
            if (!s_Instance) {
                s_Instance = new EventBus();
            }
            return s_Instance;
        }

        static void Shutdown() {
            if (s_Instance) {
                delete s_Instance;
                s_Instance = nullptr;
            }
        }

    private:
        static EventBus* s_Instance;
    };

} // namespace Nyxty