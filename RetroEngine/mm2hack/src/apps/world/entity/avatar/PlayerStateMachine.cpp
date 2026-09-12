#include "pch.h"

#include "PlayerStateMachine.h"

#include <cassert>
#include <exception>

#include "AvatarStatus.h"
#include "core/assembly/StateProvider.h"
#include "core/save/StateIO.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "PlayerParams.h"
#include "states/BrakeRunState.h"
#include "states/DashingState.h"
#include "states/HoveringState.h"
#include "states/LadderingState.h"
#include "states/LandingState.h"
#include "states/LaunchRunState.h"
#include "states/RunningState.h"
#include "states/SlidingState.h"
#include "states/StandingState.h"

namespace mm2hack::apps::world::entity::avatar
{
    namespace
    {
        bool IsRegisteredStatus(AvatarStatus status) noexcept
        {
            switch (status)
            {
            case AvatarStatus::Standing:
            case AvatarStatus::LaunchRun:
            case AvatarStatus::BrakeRun:
            case AvatarStatus::Running:
            case AvatarStatus::Hovering:
            case AvatarStatus::Landing:
            case AvatarStatus::Laddering:
            case AvatarStatus::Sliding:
            case AvatarStatus::Dashing:
                return true;
            default:
                return false;
            }
        }
    }

    bool PlayerStateMachineState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() &&
            writer.WriteI32(static_cast<std::int32_t>(status)) &&
            writer.WriteI32(static_cast<std::int32_t>(next_status)) &&
            writer.WriteU8(sliding_elapsed_frames) &&
            writer.WriteU8(dashing_elapsed_frames) &&
            writer.WriteI32(static_cast<std::int32_t>(dashing_direction)) &&
            writer.WriteBool(dash_jump_active);
    }

    bool PlayerStateMachineState::Load(core::save::StateReader& reader)
    {
        PlayerStateMachineState loaded{};
        std::int32_t encoded_status{};
        std::int32_t encoded_next_status{};
        std::int32_t encoded_direction{};
        if (!reader.ReadI32(encoded_status) ||
            !reader.ReadI32(encoded_next_status) ||
            !reader.ReadU8(loaded.sliding_elapsed_frames) ||
            !reader.ReadU8(loaded.dashing_elapsed_frames) ||
            !reader.ReadI32(encoded_direction) ||
            !reader.ReadBool(loaded.dash_jump_active))
        {
            return false;
        }
        loaded.status = static_cast<AvatarStatus>(encoded_status);
        loaded.next_status = static_cast<AvatarStatus>(encoded_next_status);
        loaded.dashing_direction = static_cast<AvatarDirection>(encoded_direction);
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool PlayerStateMachineState::IsValid() const noexcept
    {
        return IsRegisteredStatus(status) && IsRegisteredStatus(next_status) &&
            (dashing_direction == AvatarDirection::Left ||
             dashing_direction == AvatarDirection::Right);
    }

    PlayerStateMachine::PlayerStateMachine()
    {
        registerState_(std::make_unique<states::StandingState>());
        registerState_(std::make_unique<states::RunningState>());
        registerState_(std::make_unique<states::HoveringState>());
        registerState_(std::make_unique<states::LaunchRunState>());
        registerState_(std::make_unique<states::BrakeRunState>());
        registerState_(std::make_unique<states::LadderingState>());
        registerState_(std::make_unique<states::LandingState>());
        registerState_(std::make_unique<states::SlidingState>());
        registerState_(std::make_unique<states::DashingState>());
    }

    void PlayerStateMachine::Update(PlayerContext& cx, core::assembly::StateProvider* input, const PlayerTuning& tuning, double dt)
    {
        _next_status = findState_(_status).Update(cx, input, tuning, dt);
    }

    void PlayerStateMachine::CommitTransition(
        PlayerContext& cx,
        core::assembly::StateProvider* input,
        const PlayerTuning& tuning)
    {
        if (_next_status == _status)
        {
            return;
        }

        findState_(_status).OnExit(cx, input, tuning);
        _status = _next_status;
        findState_(_status).OnEnter(cx, input, tuning);
    }

    void PlayerStateMachine::TickAnimation(AnimeContext& ax, core::assembly::StateProvider* input, const PlayerTuning& tuning, double dt)
    {
        findState_(_status).TickAnimationOnly(ax, input, tuning, dt);
    }

    PlayerStateMachineState PlayerStateMachine::CaptureState() const noexcept
    {
        const auto& sliding = static_cast<const states::SlidingState&>(
            findState_(AvatarStatus::Sliding));
        const auto& dashing = static_cast<const states::DashingState&>(
            findState_(AvatarStatus::Dashing));
        const auto& hovering = static_cast<const states::HoveringState&>(
            findState_(AvatarStatus::Hovering));
        return PlayerStateMachineState{
            .status = _status,
            .next_status = _next_status,
            .sliding_elapsed_frames = sliding.ElapsedFrames(),
            .dashing_elapsed_frames = dashing.ElapsedFrames(),
            .dashing_direction = dashing.Direction(),
            .dash_jump_active = hovering.DashJumpActive(),
        };
    }

    bool PlayerStateMachine::RestoreState(const PlayerStateMachineState& state) noexcept
    {
        if (!state.IsValid())
        {
            return false;
        }

        auto& sliding = static_cast<states::SlidingState&>(
            findState_(AvatarStatus::Sliding));
        auto& dashing = static_cast<states::DashingState&>(
            findState_(AvatarStatus::Dashing));
        auto& hovering = static_cast<states::HoveringState&>(
            findState_(AvatarStatus::Hovering));
        if (!dashing.RestoreState(state.dashing_elapsed_frames, state.dashing_direction))
        {
            return false;
        }
        sliding.RestoreState(state.sliding_elapsed_frames);
        hovering.RestoreState(state.dash_jump_active);
        _status = state.status;
        _next_status = state.next_status;
        return true;
    }

    void PlayerStateMachine::registerState_(std::unique_ptr<IPlayerState> state)
    {
        assert(state != nullptr && "Cannot register a null player state.");
        if (state == nullptr)
        {
            std::terminate();
        }

        const AvatarStatus id = state->Id();
        const auto [it, inserted] = _states.try_emplace(id, std::move(state));
        (void)it;
        assert(inserted && "A player state with the same ID is already registered.");
        if (!inserted)
        {
            std::terminate();
        }
    }

    IPlayerState& PlayerStateMachine::findState_(AvatarStatus status) noexcept
    {
        const auto it = _states.find(status);
        assert(it != _states.end() && "The requested player state is not registered.");
        if (it == _states.end() || it->second == nullptr)
        {
            std::terminate();
        }
        return *it->second;
    }

    const IPlayerState& PlayerStateMachine::findState_(AvatarStatus status) const noexcept
    {
        const auto it = _states.find(status);
        assert(it != _states.end() && "The requested player state is not registered.");
        if (it == _states.end() || it->second == nullptr)
        {
            std::terminate();
        }
        return *it->second;
    }
}
