#pragma once

namespace serenity
{
    // Simple timer class (using std::chrono).
    class Timer
    {
      public:
        explicit Timer() = default;
        ~Timer() = default;

        void tick();

        float get_delta_time() const;

      private:
        std::chrono::high_resolution_clock m_clock{};

        std::chrono::time_point<std::chrono::high_resolution_clock> m_frame_start_time{};
        std::chrono::time_point<std::chrono::high_resolution_clock> m_frame_end_time{};
    };
} // namespace serenity