#include "core/events/Event.h"
#include <chrono>

namespace Nyxty {

    u64 Event::GetTimestamp() {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        return duration_cast<microseconds>(now.time_since_epoch()).count();
    }

} // namespace Nyxty