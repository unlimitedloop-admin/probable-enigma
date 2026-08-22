//==============================================================================
//
//  Project: mm2hack
//  PlayerEnvironmentController.h
//
//  Detects the player's environment and selects its physics tuning.
//
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/world/entity/common/FrameGate.h"
#include "PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar
{
    /// Result of evaluating the player's environment for one update.
    struct PlayerEnvironmentUpdate final
    {
        PlayerEnvironment previous{ PlayerEnvironment::Normal };
        PlayerEnvironment current{ PlayerEnvironment::Normal };
        bool skipPhysics{ false };

        [[nodiscard]] bool EnteredWater() const noexcept
        {
            return previous == PlayerEnvironment::Normal &&
                   current == PlayerEnvironment::Underwater;
        }
    };

    /// Owns environment detection, environment-specific tuning, and physics gating.
    class PlayerEnvironmentController final
    {
    public:
        PlayerEnvironmentController() = default;

        void SetTuning(const PlayerTuning& tuning);
        [[nodiscard]] PlayerEnvironmentUpdate Update(
            const systems::physics::ITerrainProbe* terrainProbe,
            const foundation::math::Vec2& probePoint) noexcept;

        [[nodiscard]] const PlayerTuning& CurrentTuning() const noexcept;
        [[nodiscard]] const PlayerProbes& ProbeOffsets() const noexcept;

    private:
        [[nodiscard]] PlayerEnvironment detectEnvironment_(
            const systems::physics::ITerrainProbe* terrainProbe,
            const foundation::math::Vec2& probePoint) const noexcept;
        [[nodiscard]] bool shouldSkipPhysics_() noexcept;

    private:
        PlayerTuning _normal_tuning{};
        PlayerTuning _underwater_tuning{};
        PlayerEnvironment _environment{ PlayerEnvironment::Normal };
        common::FrameGate _underwater_physics_gate{};
    };
}
