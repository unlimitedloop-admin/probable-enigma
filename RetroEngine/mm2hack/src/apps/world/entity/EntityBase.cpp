#include "pch.h"

#include "EntityBase.h"

#include <cmath>
#include <cstdlib>
#include "apps/foundation/math/CoordinateTypes.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity
{
    bool EntityKinematicState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() &&
            writer.WriteF64(position.x) && writer.WriteF64(position.y) &&
            writer.WriteF64(velocity.x) && writer.WriteF64(velocity.y);
    }

    bool EntityKinematicState::Load(core::save::StateReader& reader)
    {
        EntityKinematicState loaded{};
        if (!reader.ReadF64(loaded.position.x) ||
            !reader.ReadF64(loaded.position.y) ||
            !reader.ReadF64(loaded.velocity.x) ||
            !reader.ReadF64(loaded.velocity.y) ||
            !loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool EntityKinematicState::IsValid() const noexcept
    {
        constexpr double kMaximumCoordinate = 1'000'000.0;
        constexpr double kMaximumVelocity = 100'000.0;
        return
            std::isfinite(position.x) && std::abs(position.x) <= kMaximumCoordinate &&
            std::isfinite(position.y) && std::abs(position.y) <= kMaximumCoordinate &&
            std::isfinite(velocity.x) && std::abs(velocity.x) <= kMaximumVelocity &&
            std::isfinite(velocity.y) && std::abs(velocity.y) <= kMaximumVelocity;
    }

    bool TimedEffectEntityState::Save(
        core::save::StateWriter& writer,
        std::int32_t maximum_ticks) const
    {
        return IsValid(maximum_ticks) && kinematic.Save(writer) &&
            writer.WriteI32(base_texture) && writer.WriteI32(elapsed_ticks);
    }

    bool TimedEffectEntityState::Load(
        core::save::StateReader& reader,
        std::int32_t maximum_ticks)
    {
        TimedEffectEntityState loaded{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadI32(loaded.base_texture) ||
            !reader.ReadI32(loaded.elapsed_ticks) ||
            !loaded.IsValid(maximum_ticks))
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool TimedEffectEntityState::IsValid(std::int32_t maximum_ticks) const noexcept
    {
        return kinematic.IsValid() &&
            base_texture >= 0 && base_texture <= 65'535 &&
            maximum_ticks > 0 && elapsed_ticks >= 0 && elapsed_ticks < maximum_ticks;
    }

    EntityKinematicState EntityBase::CaptureKinematicState() const noexcept
    {
        return EntityKinematicState{ pos, vel };
    }

    bool EntityBase::RestoreKinematicState(const EntityKinematicState& state) noexcept
    {
        if (!state.IsValid())
        {
            return false;
        }
        pos = state.position;
        vel = state.velocity;
        _alive = true;
        return true;
    }
}
