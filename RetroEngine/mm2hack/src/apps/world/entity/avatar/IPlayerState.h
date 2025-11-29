//==============================================================================
// 
//  Project: mm2hack
//  IPlayerState.h
// 
//  Interface for player state behavior.
// 
//==============================================================================
#pragma once

#include "AvatarStatus.h"
#include "PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar
{
    struct PlayerContext;
}

namespace mm2hack::core::assembly
{
    class StateProvider;
}

namespace mm2hack::apps::world::entity::avatar
{
    using core::assembly::StateProvider;

    // Interface for take on player state behavior
    struct IPlayerState
    {
        virtual ~IPlayerState() = default;
        virtual AvatarStatus Id() const noexcept = 0;
        virtual void OnEnter(PlayerContext&) {}
        virtual void OnExit(PlayerContext&) {}
        // Update and return the **next state ID** (same ID if continuing)
        virtual AvatarStatus Update(PlayerContext&, StateProvider*, const PlayerTuning&, double dt) = 0;
    };
}