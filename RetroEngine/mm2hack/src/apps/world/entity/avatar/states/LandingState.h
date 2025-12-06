//==============================================================================
// 
//  Project: mm2hack
//  LandingState.h
// 
//  Operations when the player lands on the ground.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/states/GroundBaseState.h"

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

    // Player state: Landing (just landed on the ground)
    struct LandingState final : GroundBaseState
    {
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/) override;
    };
}