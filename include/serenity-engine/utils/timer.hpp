#pragma once

namespace serenity
{
    // Simple timer class (using std::chrono).
    class Timer
    {
      public:
        explicit Timer() = default;
        ~Timer() = default;
       
        // Call this at the end of the frame (else negative delta times can occur).
        void tick();

        // Note : Call this before the tick function.
        float get_delta_time() const;

      private:
        std::chrono::high_resolution_clock m_clock{};

        std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time{};
        std::chrono::time_point<std::chrono::high_resolution_clock> m_end_time{};
    };
} // namespace serenity