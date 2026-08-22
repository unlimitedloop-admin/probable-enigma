#include "pch.h"

#include "PlayerStateMachine.h"

#include "AvatarStatus.h"
#include "core/assembly/StateProvider.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "PlayerParams.h"
#include "states/BrakeRunState.h"
#include "states/HoveringState.h"
#include "states/LadderingState.h"
#include "states/LandingState.h"
#include "states/LaunchRunState.h"
#include "states/RunningState.h"
#include "states/StandingState.h"

namespace mm2hack::apps::world::entity::avatar
{
    PlayerStateMachine::PlayerStateMachine()
    {
        _states[0] = std::make_unique<states::StandingState>();
        _states[1] = std::make_unique<states::RunningState>();
        _states[2] = std::make_unique<states::HoveringState>();
        _states[3] = std::make_unique<states::LaunchRunState>();
        _states[4] = std::make_unique<states::BrakeRunState>();
        _states[5] = std::make_unique<states::LadderingState>();
        _states[6] = std::make_unique<states::LandingState>();
    }

    void PlayerStateMachine::Update(
        PlayerContext& cx,
        core::assembly::StateProvider* input,
        const PlayerTuning& tuning,
        double dt)
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

    void PlayerStateMachine::TickAnimation(
        AnimeContext& ax,
        core::assembly::StateProvider* input,
        const PlayerTuning& tuning,
        double dt)
    {
        findState_(_status).TickAnimationOnly(ax, input, tuning, dt);
    }

    IPlayerState& PlayerStateMachine::findState_(AvatarStatus status) noexcept
    {
        switch (status)
        {
        case AvatarStatus::Running:   return *_states[1];
        case AvatarStatus::Hovering:  return *_states[2];
        case AvatarStatus::LaunchRun: return *_states[3];
        case AvatarStatus::BrakeRun:  return *_states[4];
        case AvatarStatus::Laddering: return *_states[5];
        case AvatarStatus::Landing:   return *_states[6];
        case AvatarStatus::Standing:
        default:                      return *_states[0];
        }
    }
}
