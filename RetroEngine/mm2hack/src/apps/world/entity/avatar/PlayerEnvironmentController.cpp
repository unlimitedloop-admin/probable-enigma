#include "pch.h"

#include "PlayerEnvironmentController.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/TileAttribute.h"
#include "PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar
{
    void PlayerEnvironmentController::SetTuning(const PlayerTuning& tuning)
    {
        _normal_tuning = tuning;
        _underwater_tuning = MakeUnderwaterTuning(tuning);
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

        constexpr int kSkipInterval = 5;
        return _underwater_physics_gate.step(kSkipInterval);
    }
}
