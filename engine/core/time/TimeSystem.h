#pragma once

#include "core/foundation/Base.h"
#include "core/foundation/Core.h"

#include <chrono>

namespace Nyxty {

    class NYXTY_CORE_API TimeSystem {
    public:
        void Start();
        void Tick();

        void SetFixedDeltaSeconds(double seconds);
        void SetTimeScale(double scale);

        double GetDeltaSeconds() const { return m_DeltaSeconds; }
        double GetUnscaledDeltaSeconds() const { return m_UnscaledDeltaSeconds; }
        double GetFixedDeltaSeconds() const { return m_FixedDeltaSeconds; }
        double GetElapsedSeconds() const { return m_ElapsedSeconds; }
        double GetTimeScale() const { return m_TimeScale; }
        u64 GetFrameIndex() const { return m_FrameIndex; }

    private:
        using Clock = std::chrono::steady_clock;

        Clock::time_point m_LastTick{};
        double m_DeltaSeconds{ 0.0 };
        double m_UnscaledDeltaSeconds{ 0.0 };
        double m_FixedDeltaSeconds{ 1.0 / 60.0 };
        double m_ElapsedSeconds{ 0.0 };
        double m_TimeScale{ 1.0 };
        u64 m_FrameIndex{ 0 };
        bool m_Running{ false };
    };

} // namespace Nyxty
