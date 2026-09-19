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
        // wall-clock seconds. `inputs` carries whatever this frame's
        // Grounded/Airborne/... signals are (see AnimationConditionInputs).
        void Tick(const AnimationConditionInputs& inputs) noexcept;

        [[nodiscard]] int CurrentTile() const noexcept;
        // Whether the current state allows the entity's own locomotion (see
        // AnimationState::allow_movement). Permissive (true) with no attached
        // definition, matching the rest of this class's null-safety.
        [[nodiscard]] bool AllowsMovement() const noexcept;
        // Multiplies the caller's own base horizontal speed (see
        // AnimationState::move_speed_multiplier). 1.0 with no attached definition.
        [[nodiscard]] double MoveSpeedMultiplier() const noexcept;
        // 0.0 normally; non-zero only immediately after a Tick() where a
        // transition carrying a jump_impulse fired (see AnimationTransition::
        // jump_impulse) -- read this right after calling Tick(), same frame.
        [[nodiscard]] double LastJumpImpulse() const noexcept { return _pending_jump_impulse; }
        // Empty normally; holds the firing transition's projectile_spawns
        // immediately after a Tick() where one fired -- read this right after
        // calling Tick(), same frame (see AnimationTransition::projectile_spawns).
        [[nodiscard]] const std::vector<ProjectileSpawnSpec>& LastProjectileSpawns() const noexcept
        {
            return _pending_projectile_spawns;
        }

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
        double _pending_jump_impulse{ 0.0 }; // See LastJumpImpulse(); reset every Tick()
        std::vector<ProjectileSpawnSpec> _pending_projectile_spawns{}; // See LastProjectileSpawns(); reset every Tick()
    };
}
