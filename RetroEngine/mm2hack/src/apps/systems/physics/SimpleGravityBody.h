//==============================================================================
//
//  Project: mm2hack
//  SimpleGravityBody.h
//
//  Lightweight vertical-only physics for entities that don't need the
//  player's full Probes-based terrain interaction (wall slides, ladders,
//  dashing, ...) -- just "fall straight down, land on solid ground, stop."
//
//  Deliberately NOT shared with PlayerEntity/MovementAbilities.h: the
//  player's gravity code is tightly coupled to PlayerContext/Probes and
//  already tuned and working, so retrofitting it onto this instead of the
//  other way around risks regressing it for no benefit. Uses the same
//  per-frame constant convention as PlayerTuning::gravity/terminalVelocity
//  (a plain px-per-frame value, not scaled by dt) so the two still *feel*
//  consistent even though the code paths are independent.
//
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::systems::physics
{
    class ITerrainProbe;

    struct SimpleGravityBodyState final
    {
        double vertical_velocity{ 0.0 };
        bool on_ground{ false };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    class SimpleGravityBody final
    {
    public:
        SimpleGravityBody() noexcept = default;
        // `gravity_per_frame`/`terminal_velocity_per_frame`: same units as
        // PlayerTuning::gravity/terminalVelocity, so a caller wanting "the same
        // feel as the player" can just copy those constants; a per-kind
        // multiplier on top of that is the caller's job (see EnemyEntity's
        // gravity_scale), not this class's.
        SimpleGravityBody(double gravity_per_frame, double terminal_velocity_per_frame) noexcept;

        // Advances one frame: applies gravity, then probes the tile directly
        // below (`foot_x`, `current_bottom_y` + this frame's fall distance). On
        // landing, snaps to the tile's top edge and zeroes vertical velocity.
        // Returns the resolved bottom-edge Y (the caller derives pos.y from its
        // own half-size). `terrain` may be null -- returns `current_bottom_y`
        // unchanged and leaves IsOnGround() at whatever it last was.
        [[nodiscard]] double Tick(
            const ITerrainProbe* terrain, double foot_x, double current_bottom_y) noexcept;

        // Sets vertical velocity to `impulse` directly (negative = upward, same
        // sign convention as PlayerTuning::jumpImpulse) and clears the on-ground
        // flag. Meant to be called the instant an entity enters a jump/airborne
        // animation state (see EnemyEntity, which drives this off
        // AnimationStatePlayer's transition-carried jump_impulse).
        void Jump(double impulse) noexcept
        {
            _vel_y = impulse;
            _on_ground = false;
        }

        [[nodiscard]] bool IsOnGround() const noexcept { return _on_ground; }
        [[nodiscard]] double VerticalVelocity() const noexcept { return _vel_y; }
        // Configuration accessors, for callers that need to persist the already-
        // scaled per-frame values alongside this component's live state (see
        // EnemyEntityState::gravity_per_frame/terminal_velocity_per_frame).
        [[nodiscard]] double GravityPerFrame() const noexcept { return _gravity; }
        [[nodiscard]] double TerminalVelocityPerFrame() const noexcept { return _terminal_velocity; }

        [[nodiscard]] SimpleGravityBodyState CaptureState() const noexcept
        {
            return SimpleGravityBodyState{ _vel_y, _on_ground };
        }
        bool RestoreState(const SimpleGravityBodyState& state) noexcept
        {
            if (!state.IsValid()) return false;
            _vel_y = state.vertical_velocity;
            _on_ground = state.on_ground;
            return true;
        }

    private:
        double _gravity{ 0.0 };
        double _terminal_velocity{ 0.0 };
        double _vel_y{ 0.0 };
        bool _on_ground{ false };
    };
}
