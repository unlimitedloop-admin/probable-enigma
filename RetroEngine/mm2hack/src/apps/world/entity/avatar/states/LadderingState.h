//==============================================================================
// 
//  Project: mm2hack
//  LadderingState.h
// 
//  Operation for the "Laddering" avatar state.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/IPlayerState.h"

#include <string>
#include <utility>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"

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
    using foundation::math::Vec2;

    // Player state: Laddering (on ladder)
    class LadderingState final : public IPlayerState
    {
    public:
        // Get state ID
        AvatarStatus Id() const noexcept override;
        // Called when entering the state
        void OnEnter(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) override;
        // Called when exiting the state
        void OnExit(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) override;
        // Update state and return next state ID
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/) override;
        // Animation only tick
        void TickAnimationOnly(AnimeContext& ax, StateProvider* in, const PlayerTuning& t, double dt) override;

    private:
        // Check if the player is still on the ladder
        bool isOnLadder_(const PlayerContext& cx, const PlayerTuning& t) const noexcept;
        // Snap player X position to ladder center
        void snapToLadderCenter_(PlayerContext& cx, const PlayerTuning& t) const noexcept;
        // Check if should rise to ground
        bool shouldRisingToGround_(const PlayerContext& cx) const;
        // Perform rising to ground action
        void doRisingToGround_(PlayerContext& cx) const;
        // Build grab candidate positions
        void buildGrabCandidates_(Vec2 out[9], const PlayerContext& cx, const PlayerTuning& t) const noexcept;

        std::pair<int, bool> computeInputAndTopEmpty_(const PlayerContext& cx, StateProvider* in) const noexcept;

    private:
        const std::wstring kClassName{ L"LadderingState" };
        static constexpr double kSnapEpsBase = 1.0;
    };
}