//==============================================================================
// 
//  Project: mm2hack
//  HoveringState.h
// 
//  Operation for the "Hovering" avatar state.
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
    // Player state: Running (on ground, moving)
    struct HoveringState final : IPlayerState
    {
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, const InputSnapshot& in, const PlayerTuning& t, double /*dt*/) override;
    };
}