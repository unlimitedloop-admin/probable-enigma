//==============================================================================
// 
//  Project: mm2hack
//  RunningState.h
// 
//  Operation for the "Running" avatar state.
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

    // Player state: Running (on ground, moving)
    struct RunningState final : GroundBaseState
    {
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/) override;
    };
}