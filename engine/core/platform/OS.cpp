#include "OS.h"
#include <thread>

namespace Platform {
    void SleepMs(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
}
