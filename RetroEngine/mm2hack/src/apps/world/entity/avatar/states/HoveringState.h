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
    using core::assembly::StateProvider;

    // Player state: Running (on ground, moving)
    class HoveringState final : public IPlayerState
    {
    public:
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/) override;

    private:
        bool tryEnterLadder_(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) const;

    private:
        const std::wstring kClassName{ L"HoveringState" };
    };
}