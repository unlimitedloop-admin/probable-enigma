//==============================================================================
// 
//  Project: mm2hack
//  LandingState.h
// 
//  Operations when the player lands on the ground.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/IPlayerState.h"

#include "apps/world/entity/avatar/AvatarStatus.h"

namespace mm2hack::apps::world::entity::avatar
{
    struct PlayerContext;
    struct InputSnapshot;
    struct PlayerTuning;
}

namespace mm2hack::apps::world::entity::avatar::states
{
    // Player state: Landing (just landed on the ground)
    struct LandingState final : IPlayerState
    {
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, const InputSnapshot& in, const PlayerTuning& t, double /*dt*/) override;
    };
}