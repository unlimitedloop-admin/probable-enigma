//==============================================================================
// 
//  Project: mm2hack
//  FrameGate.h
// 
//  Periodic frame gate for frame-based processing.
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::world::entity::common
{
    class FrameGate
    {
    public:
        // Advances the counter and returns true at the specified interval.
        [[nodiscard]] bool step(int interval) noexcept
        {
            if (interval <= 0)
            {
                return false;
            }

            ++_counter;

            if (_counter < interval)
            {
                return false;
            }

            _counter = 0;
            return true;
        }

        // Resets the frame counter.
        void reset() noexcept
        {
            _counter = 0;
        }

    private:
        int _counter{ 0 };
    };
}