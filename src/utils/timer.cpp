#include "serenity-engine/utils/timer.hpp"

namespace serenity
{
    void Timer::tick()
    {
        m_frame_end_time = m_frame_start_time;
        m_frame_start_time = m_clock.now();
    }

    float Timer::get_delta_time() const
    {
        return static_cast<float>(
            std::chrono::duration_cast<std::chrono::milliseconds>(m_frame_end_time - m_frame_start_time).count());
    }
} // namespace serenity