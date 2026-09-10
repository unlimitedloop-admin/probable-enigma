#include "pch.h"

#include "AnimeStepper.h"

#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::common
{
    bool AnimeStepperState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() && writer.WriteI32(tick) &&
            writer.WriteI32(frame) && writer.WriteI32(loops);
    }

    bool AnimeStepperState::Load(core::save::StateReader& reader)
    {
        AnimeStepperState loaded{};
        if (!reader.ReadI32(loaded.tick) ||
            !reader.ReadI32(loaded.frame) ||
            !reader.ReadI32(loaded.loops) ||
            !loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool AnimeStepperState::IsValid() const noexcept
    {
        constexpr std::int32_t kMaximumCounter = 1'000'000;
        return tick >= 0 && tick <= kMaximumCounter &&
            frame >= 0 && frame <= kMaximumCounter &&
            loops >= 0 && loops <= kMaximumCounter;
    }

    AnimeStepperState AnimeStepper::CaptureState() const noexcept
    {
        return AnimeStepperState{ tick, frame, loops };
    }

    bool AnimeStepper::RestoreState(const AnimeStepperState& state) noexcept
    {
        if (!state.IsValid())
        {
            return false;
        }
        tick = state.tick;
        frame = state.frame;
        loops = state.loops;
        return true;
    }
}
