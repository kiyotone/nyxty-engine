#include "core/time/TimeSystem.h"

#include <algorithm>

namespace Nyxty {

void TimeSystem::Start() {
    m_LastTick = Clock::now();
    m_DeltaSeconds = 0.0;
    m_UnscaledDeltaSeconds = 0.0;
    m_ElapsedSeconds = 0.0;
    m_FrameIndex = 0;
    m_Running = true;
}

void TimeSystem::Tick() {
    if (!m_Running) {
        Start();
        return;
    }

    const auto now = Clock::now();
    m_UnscaledDeltaSeconds = std::chrono::duration<double>(now - m_LastTick).count();
    m_DeltaSeconds = m_UnscaledDeltaSeconds * m_TimeScale;
    m_ElapsedSeconds += m_DeltaSeconds;
    m_LastTick = now;
    ++m_FrameIndex;
}

void TimeSystem::SetFixedDeltaSeconds(double seconds) {
    m_FixedDeltaSeconds = std::max(seconds, 0.0001);
}

void TimeSystem::SetTimeScale(double scale) {
    m_TimeScale = std::max(scale, 0.0);
}

} // namespace Nyxty
