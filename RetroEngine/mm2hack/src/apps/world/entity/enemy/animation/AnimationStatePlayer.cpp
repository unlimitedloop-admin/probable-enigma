#include "pch.h"

#include "AnimationStatePlayer.h"

namespace mm2hack::apps::world::entity::enemy::animation
{
    AnimationStatePlayer::AnimationStatePlayer(const EnemyAnimationDef& def) noexcept
        : _def(&def)
    {
        const int initial = def.FindStateIndex(def.initial_state);
        _state_index = (initial >= 0) ? initial : 0;
    }

    void AnimationStatePlayer::Tick(const AnimationConditionInputs& inputs) noexcept
    {
        _pending_jump_impulse = 0.0;
        _pending_projectile_spawns.clear();
        _pending_counter_increment_requested = false;

        if (_def == nullptr || _def->states.empty() ||
            _state_index < 0 || _state_index >= static_cast<int>(_def->states.size()))
        {
            return;
        }

        const AnimationState& state = _def->states[static_cast<std::size_t>(_state_index)];
        ++_state_elapsed;

        // Advance the clip. `clip_finished_this_tick` is a one-tick pulse, true
        // only on the exact tick a non-looping clip's last frame completes its
        // wait -- not "every tick while sitting on the last frame" (which would
        // fire ClipFinished repeatedly).
        bool clip_finished_this_tick = false;
        if (!state.clip.frames.empty() &&
            _frame_index >= 0 && _frame_index < static_cast<int>(state.clip.frames.size()))
        {
            const AnimationFrame& frame = state.clip.frames[static_cast<std::size_t>(_frame_index)];
            if (frame.wait_frames != AnimationFrame::kHoldFrames)
            {
                ++_frame_elapsed;
                if (_frame_elapsed >= frame.wait_frames)
                {
                    _frame_elapsed = 0;
                    const bool is_last_frame =
                        _frame_index == static_cast<int>(state.clip.frames.size()) - 1;
                    if (!is_last_frame)
                    {
                        ++_frame_index;
                    }
                    else if (state.clip.loop)
                    {
                        _frame_index = 0;
                    }
                    else
                    {
                        clip_finished_this_tick = true; // stays on the last frame
                    }
                }
            }
        }

        // Evaluate this state's transitions in order; the first whose condition
        // fires wins.
        for (const auto& transition : state.transitions)
        {
            bool fires = false;
            switch (transition.condition)
            {
            case AnimationCondition::Timer:
                fires = _state_elapsed >= transition.param_frames;
                break;
            case AnimationCondition::ClipFinished:
                fires = clip_finished_this_tick;
                break;
            case AnimationCondition::Grounded:
                fires = inputs.grounded;
                break;
            case AnimationCondition::Airborne:
                fires = !inputs.grounded;
                break;
            case AnimationCondition::PlayerNear:
                fires = inputs.player_dx <= transition.param_x &&
                    inputs.player_dy <= transition.param_y;
                break;
            default:
                fires = false;
                break;
            }

            if (fires && transition.required_parity != CounterParity::Any)
            {
                const bool counter_is_even = (inputs.shared_counter % 2) == 0;
                const bool wants_even = transition.required_parity == CounterParity::Even;
                fires = counter_is_even == wants_even;
            }

            if (!fires)
            {
                continue;
            }

            const int target = _def->FindStateIndex(transition.to_state);
            if (target >= 0)
            {
                _state_index = target;
                _frame_index = 0;
                _frame_elapsed = 0;
                _state_elapsed = 0;
                _pending_jump_impulse = transition.jump_impulse;
                _pending_projectile_spawns = transition.projectile_spawns;
                _pending_counter_increment_requested = transition.increments_shared_counter;
            }
            break;
        }
    }

    int AnimationStatePlayer::CurrentTile() const noexcept
    {
        if (_def == nullptr ||
            _state_index < 0 || _state_index >= static_cast<int>(_def->states.size()))
        {
            return 0;
        }

        const auto& frames = _def->states[static_cast<std::size_t>(_state_index)].clip.frames;
        if (frames.empty() || _frame_index < 0 || _frame_index >= static_cast<int>(frames.size()))
        {
            return 0;
        }
        return frames[static_cast<std::size_t>(_frame_index)].tile;
    }

    bool AnimationStatePlayer::AllowsMovement() const noexcept
    {
        if (_def == nullptr ||
            _state_index < 0 || _state_index >= static_cast<int>(_def->states.size()))
        {
            return true;
        }
        return _def->states[static_cast<std::size_t>(_state_index)].allow_movement;
    }

    double AnimationStatePlayer::MoveSpeedMultiplier() const noexcept
    {
        if (_def == nullptr ||
            _state_index < 0 || _state_index >= static_cast<int>(_def->states.size()))
        {
            return 1.0;
        }
        return _def->states[static_cast<std::size_t>(_state_index)].move_speed_multiplier;
    }

    bool AnimationStatePlayer::TracksPlayerFacing() const noexcept
    {
        if (_def == nullptr ||
            _state_index < 0 || _state_index >= static_cast<int>(_def->states.size()))
        {
            return true;
        }
        return _def->states[static_cast<std::size_t>(_state_index)].track_player_facing;
    }

    bool AnimationStatePlayer::DeflectsAttacks() const noexcept
    {
        if (_def == nullptr ||
            _state_index < 0 || _state_index >= static_cast<int>(_def->states.size()))
        {
            return false;
        }
        return _def->states[static_cast<std::size_t>(_state_index)].deflects_attacks;
    }

    bool AnimationStatePlayer::RestoreState(
        int state_index, int frame_index, int frame_elapsed, int state_elapsed,
        const EnemyAnimationDef& def) noexcept
    {
        if (def.states.empty() ||
            state_index < 0 || state_index >= static_cast<int>(def.states.size()))
        {
            return false;
        }

        const auto& frames = def.states[static_cast<std::size_t>(state_index)].clip.frames;
        if (frames.empty() || frame_index < 0 || frame_index >= static_cast<int>(frames.size()) ||
            frame_elapsed < 0 || frame_elapsed > 36'000 ||
            state_elapsed < 0 || state_elapsed > 36'000)
        {
            return false;
        }

        _def = &def;
        _state_index = state_index;
        _frame_index = frame_index;
        _frame_elapsed = frame_elapsed;
        _state_elapsed = state_elapsed;
        return true;
    }
}
