//==============================================================================
// 
//  Project: mm2hack
//  FrameGate.h
// 
//  Periodic frame gate for frame-based processing.
// 
//==============================================================================
#pragma once

#include <cstdint>

#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::common
{
    struct FrameGateState final
    {
        std::int32_t counter{};

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid(std::int32_t interval) const noexcept;
    };

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

        [[nodiscard]] FrameGateState CaptureState() const noexcept;
        bool RestoreState(const FrameGateState& state, std::int32_t interval) noexcept;

    private:
        int _counter{ 0 };
    };
}
