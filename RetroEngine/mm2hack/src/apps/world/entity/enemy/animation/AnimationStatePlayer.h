//==============================================================================
//
//  Project: mm2hack
//  AnimationStatePlayer.h
//
//  Steps one entity through its EnemyAnimationDef: advances the active
//  state's clip frame-by-frame and switches states when a transition's
//  condition fires. Doesn't own the definition -- the definition lives in
//  EnemyDefinitionCatalog for the whole stage's lifetime, well past any one
//  entity using it.
//
//==============================================================================
#pragma once

#include "AnimationTypes.h"

namespace mm2hack::apps::world::entity::enemy::animation
{
    class AnimationStatePlayer final
    {
    public:
        AnimationStatePlayer() noexcept = default;
        explicit AnimationStatePlayer(const EnemyAnimationDef& def) noexcept;

        // Advances one tick. Assumed to be called once per game frame (60fps),
        // matching every other frame-stepped effect in this codebase (see
        // ChargeEffectEntity) -- durations in the data are frame counts, not
        // wall-clock seconds.
        void Tick() noexcept;

        [[nodiscard]] int CurrentTile() const noexcept;

        // Save/restore. Restoring re-attaches to `def` (not persisted itself --
        // it's resolved by EnemyKind at reconstruction time, same as a sprite id).
        [[nodiscard]] int StateIndex() const noexcept { return _state_index; }
        [[nodiscard]] int FrameIndex() const noexcept { return _frame_index; }
        [[nodiscard]] int FrameElapsed() const noexcept { return _frame_elapsed; }
        [[nodiscard]] int StateElapsed() const noexcept { return _state_elapsed; }
        bool RestoreState(
            int state_index, int frame_index, int frame_elapsed, int state_elapsed,
            const EnemyAnimationDef& def) noexcept;

    private:
        const EnemyAnimationDef* _def{ nullptr };
        int _state_index{ 0 };
        int _frame_index{ 0 };
        int _frame_elapsed{ 0 };    // Ticks spent on the current frame
        int _state_elapsed{ 0 };    // Ticks spent in the current state (drives Timer)
    };
}
