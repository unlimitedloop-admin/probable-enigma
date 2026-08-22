//==============================================================================
// 
//  Project: mm2hack
//  LaunchRunState.h
// 
//  Operation for the "LaunchRun" avatar state.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/states/GroundBaseState.h"

#include <string>
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
    // Player state: LaunchRun (starting to run from standstill)
    class LaunchRunState final : public GroundBaseState
    {
    public:
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, core::assembly::StateProvider* in, const PlayerTuning& t, double /*dt*/) override;

    private:
        const std::wstring kClassName{ L"LaunchRunState" };
    };
}