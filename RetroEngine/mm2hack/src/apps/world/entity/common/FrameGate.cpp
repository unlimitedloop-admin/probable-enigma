#include "pch.h"

#include "FrameGate.h"

#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::common
{
    bool FrameGateState::Save(core::save::StateWriter& writer) const
    {
        return counter >= 0 && writer.WriteI32(counter);
    }

    bool FrameGateState::Load(core::save::StateReader& reader)
    {
        FrameGateState loaded{};
        if (!reader.ReadI32(loaded.counter) || loaded.counter < 0)
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool FrameGateState::IsValid(std::int32_t interval) const noexcept
    {
        return interval > 0 && counter >= 0 && counter < interval;
    }

    FrameGateState FrameGate::CaptureState() const noexcept
    {
        return FrameGateState{ _counter };
    }

    bool FrameGate::RestoreState(
        const FrameGateState& state,
        std::int32_t interval) noexcept
    {
        if (!state.IsValid(interval))
        {
            return false;
        }
        _counter = state.counter;
        return true;
    }
}
