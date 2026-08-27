#include "pch.h"

#include "PlayerStateMachine.h"

#include <cassert>
#include <exception>
#include <utility>
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
#include "states/SlidingState.h"
#include "states/StandingState.h"

namespace mm2hack::apps::world::entity::avatar
{
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
}
