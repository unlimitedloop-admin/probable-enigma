//==============================================================================
//
//  Project: mm2hack
//  PlayerStateMachine.h
//
//  Owns and coordinates the player's locomotion states.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "core/save/StateIO.h"
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
    struct PlayerStateMachineState final
    {
        AvatarStatus status{ AvatarStatus::Standing };
        AvatarStatus next_status{ AvatarStatus::Standing };
        std::uint8_t sliding_elapsed_frames{};
        std::uint8_t dashing_elapsed_frames{};
        AvatarDirection dashing_direction{ AvatarDirection::Right };
        bool dash_jump_active{};

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

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
        [[nodiscard]] PlayerStateMachineState CaptureState() const noexcept;
        bool RestoreState(const PlayerStateMachineState& state) noexcept;

    private:
        void registerState_(std::unique_ptr<IPlayerState> state);
        IPlayerState& findState_(AvatarStatus status) noexcept;
        const IPlayerState& findState_(AvatarStatus status) const noexcept;

    private:
        AvatarStatus _status{ AvatarStatus::Standing };
        AvatarStatus _next_status{ AvatarStatus::Standing };
        std::unordered_map<AvatarStatus, std::unique_ptr<IPlayerState>> _states{};
    };
}
