#include "pch.h"

#include "PlayerEnvironmentController.h"

#include <cstdint>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/TileAttribute.h"
#include "PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar
{
    bool PlayerEnvironmentState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() &&
            writer.WriteU8(static_cast<std::uint8_t>(environment)) &&
            underwater_physics_gate.Save(writer);
    }

    bool PlayerEnvironmentState::Load(core::save::StateReader& reader)
    {
        PlayerEnvironmentState loaded{};
        std::uint8_t encoded_environment{};
        if (!reader.ReadU8(encoded_environment) ||
            !loaded.underwater_physics_gate.Load(reader))
        {
            return false;
        }
        loaded.environment = static_cast<PlayerEnvironment>(encoded_environment);
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool PlayerEnvironmentState::IsValid() const noexcept
    {
        return (environment == PlayerEnvironment::Normal ||
                environment == PlayerEnvironment::Underwater) &&
            underwater_physics_gate.IsValid(
                PlayerEnvironmentController::kUnderwaterSkipInterval) &&
            (environment != PlayerEnvironment::Normal ||
             underwater_physics_gate.counter == 0);
    }
    void PlayerEnvironmentController::SetTuning(const PlayerTuning& tuning)
    {
        _normal_tuning = tuning;
        _underwater_tuning = make_underwater_tuning(tuning);
    }

    PlayerEnvironmentUpdate PlayerEnvironmentController::Update(
        const systems::physics::ITerrainProbe* terrainProbe,
        const foundation::math::Vec2& probePoint) noexcept
    {
        const PlayerEnvironment previous = _environment;
        _environment = detectEnvironment_(terrainProbe, probePoint);

        return PlayerEnvironmentUpdate{
            .previous = previous,
            .current = _environment,
            .skipPhysics = shouldSkipPhysics_()
        };
    }

    const PlayerTuning& PlayerEnvironmentController::CurrentTuning() const noexcept
    {
        if (_environment == PlayerEnvironment::Underwater)
        {
            return _underwater_tuning;
        }

        return _normal_tuning;
    }

    const PlayerProbes& PlayerEnvironmentController::ProbeOffsets() const noexcept
    {
        return _normal_tuning.probeOffsets;
    }

    PlayerEnvironmentState PlayerEnvironmentController::CaptureState() const noexcept
    {
        return PlayerEnvironmentState{
            _environment,
            _underwater_physics_gate.CaptureState()
        };
    }

    bool PlayerEnvironmentController::RestoreState(
        const PlayerEnvironmentState& state) noexcept
    {
        if (!state.IsValid() ||
            !_underwater_physics_gate.RestoreState(
                state.underwater_physics_gate,
                kUnderwaterSkipInterval))
        {
            return false;
        }
        _environment = state.environment;
        return true;
    }

    PlayerEnvironment PlayerEnvironmentController::detectEnvironment_(
        const systems::physics::ITerrainProbe* terrainProbe,
        const foundation::math::Vec2& probePoint) const noexcept
    {
        if (terrainProbe == nullptr)
        {
            return PlayerEnvironment::Normal;
        }

        const systems::physics::TileAttribute attribute =
            terrainProbe->AttributeAt(probePoint);

        if (systems::physics::Has(attribute, systems::physics::TileAttribute::Water))
        {
            return PlayerEnvironment::Underwater;
        }

        return PlayerEnvironment::Normal;
    }

    bool PlayerEnvironmentController::shouldSkipPhysics_() noexcept
    {
        if (_environment != PlayerEnvironment::Underwater)
        {
            _underwater_physics_gate.reset();
            return false;
        }

        return _underwater_physics_gate.step(kUnderwaterSkipInterval);
    }
}
