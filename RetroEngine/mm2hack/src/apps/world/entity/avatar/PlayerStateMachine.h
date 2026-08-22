//==============================================================================
//
//  Project: mm2hack
//  PlayerStateMachine.h
//
//  Owns and coordinates the player's locomotion states.
//
//==============================================================================
#pragma once

#include <array>
#include <memory>
#include "AvatarStatus.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "PlayerParams.h"

namespace mm2hack::core::assembly
{
    class StateProvider;
}

namespace mm2hack::apps::world::entity::avatar
{
    // State machine for the player's mutually exclusive locomotion behavior
    class PlayerStateMachine final
    {
    public:
        PlayerStateMachine();

        PlayerStateMachine(const PlayerStateMachine&) = delete;
        PlayerStateMachine& operator=(const PlayerStateMachine&) = delete;
        PlayerStateMachine(PlayerStateMachine&&) = default;
        PlayerStateMachine& operator=(PlayerStateMachine&&) = default;
        ~PlayerStateMachine() = default;

        // Evaluate the current state and retain its requested next state
        void Update(PlayerContext& cx, core::assembly::StateProvider* input, const PlayerTuning& tuning, double dt);
        // Apply the retained transition after parallel player actions are updated
        void CommitTransition(PlayerContext& cx, core::assembly::StateProvider* input, const PlayerTuning& tuning);
        // Advance animation without updating locomotion
        void TickAnimation(AnimeContext& ax, core::assembly::StateProvider* input, const PlayerTuning& tuning, double dt);

        [[nodiscard]] AvatarStatus Status() const noexcept { return _status; }

    private:
        IPlayerState& findState_(AvatarStatus status) noexcept;

    private:
        AvatarStatus _status{ AvatarStatus::Standing };
        AvatarStatus _next_status{ AvatarStatus::Standing };
        std::array<std::unique_ptr<IPlayerState>, 7> _states{};
    };
}
