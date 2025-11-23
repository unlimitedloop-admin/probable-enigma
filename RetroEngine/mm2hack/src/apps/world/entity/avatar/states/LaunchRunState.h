//==============================================================================
// 
//  Project: mm2hack
//  LaunchRunState.h
// 
//  Operation for the "LaunchRun" avatar state.
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
    // Player state: LaunchRun (starting to run from standstill)
    struct LaunchRunState final : IPlayerState
    {
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, const InputSnapshot& in, const PlayerTuning& t, double /*dt*/) override;
    };
}