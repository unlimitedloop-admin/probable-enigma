//==============================================================================
// 
//  Project: mm2hack
//  BrakeRunState.h
// 
//  State for the "BrakeRun" avatar state.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/IPlayerState.h"

#include "apps/world/entity/avatar/AvatarStatus.h"

namespace mm2hack::apps::world::entity::avatar
{
    struct PlayerContext;
    struct PlayerTuning;
}

namespace mm2hack::core::assembly
{
    class StateProvider;
}

namespace mm2hack::apps::world::entity::avatar::states
{
    using core::assembly::StateProvider;

    // Player state: BrakeRun (decelerating while running)
    struct BrakeRunState final : IPlayerState
    {
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/) override;
    };
}