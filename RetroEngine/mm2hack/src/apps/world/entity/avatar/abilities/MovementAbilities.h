//==============================================================================
// 
//  Project: mm2hack
//  MovementAbilities.h
// 
//  Movement abilities for the avatar entity.
// 
//==============================================================================
#pragma once

#include <cmath>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::abilities
{
    using foundation::math::Vec2;

    // Create GroundMoveIntent based on input and PlayerTuning
    inline GroundMoveIntent MakeInputMoveIntent(StateProvider* in, const PlayerTuning& t, AvatarStatus st)
    {
        const bool left = in->IsPressed(JPBTN::LEFT);
        const bool right = in->IsPressed(JPBTN::RIGHT);

        if (!(left ^ right))
        {
            return {}; // active=false
        }

        const int sign = left ? -1 : +1;

        double spd = 0.0;
        switch (st)
        {
        case AvatarStatus::Standing:
        case AvatarStatus::LaunchRun: spd = t.momentumStart; break;
        case AvatarStatus::Running:   spd = t.steadyRun;     break;
        default:                      spd = 0.0;             break;
        }

        return GroundMoveIntent{ sign, spd, spd > 0.0 };
    }

    // Create GroundMoveIntent based on PlayerContext and PlayerTuning (Intended for use only with BrakeRun)
    inline GroundMoveIntent MakeBrakeRunIntent(const PlayerContext& cx, const PlayerTuning& t)
    {
        const int sign = (cx.facingLR == AvatarDirection::Left) ? -1 : +1;
        return GroundMoveIntent{ sign, t.haltSpeed, t.haltSpeed > 0.0 };
    }

    // *ApplyGroundMove; X-axis movement for ground
    inline void ApplyGroundMove(PlayerContext& cx, const GroundMoveIntent& intent)
    {
        cx.vel.x = (intent.active) ? intent.speed * static_cast<double>(intent.dirSign) : 0.0;
    }

    inline void LandingMove(PlayerContext& cx, const PlayerTuning& t)
    {
        cx.vel.x = t.steadyRun * static_cast<double>(cx.facingLR);
    }

    inline void SubjectToGravityOnGround(PlayerContext& cx, const PlayerTuning& t)
    {
        cx.vel.y = t.gravity;
    }

    inline void SetJumpVelocity(Vec2& vel, double impulse)
    {
        vel.y = impulse;
    }

    // *StartJump; sets vertical speed for jump
    inline void DoJump(PlayerContext& cx, const PlayerTuning& t)
    {
        if (cx.terrain->SweepVertical(cx.probes, -0x00.01p0).hit)
        {
            // Cannot jump if there is a ceiling right above.
            return;
        }
        // Add jump impulse.
        SetJumpVelocity(cx.vel, t.jumpImpulse);
    }

    // *UpdateAirHorizontalVelocity; X-axis movement for air (do not decelerate in air)
    inline void ApplyAirControl(Vec2& vel, bool left, bool right, double accel)
    {
        if (left ^ right)
        {
            double s = left ? -1.0 : 1.0;
            vel.x = s * accel;
        }
        else
        {
            vel.x = 0.0;
        }
        // NOTE: Do not decelerate in air. (Realistic physics)
    }

    inline void ApplyGravity(Vec2& vel, double g, double terminal)
    {
        vel.y += g;
        if (vel.y > terminal) vel.y = terminal; // The max speed when falling (upper limit).
    }

    // *ApplyVerticalPhysics; Updates vertical velocity for jump and fall; applies gravity or jump cut as needed
    inline void UpdateVerticalVelocity(PlayerContext& cx, const PlayerTuning& t, bool isJump)
    {
        // Timing of falling while floating.
        if (isJump || cx.vel.y >= t.fallingThreshold)
        {
            ApplyGravity(cx.vel, t.gravity, t.terminalVelocity);
        }
        else    // Stopped jumping midway, start falling faster.
        {
            cx.vel.y = t.jumpCutVelocity;
        }
    }

    // *ApplyGravityIfGrounded; otherwise preserve vertical speed
    inline void AdjustVerticalSpeedForGravity(PlayerContext& cx, const PlayerTuning& t)
    {
        SubjectToGravityOnGround(cx, t);

        //double y = 0.0, f = 0.0;
        //y = -std::modf(cx.pos.y, &f);
        //cx.vel.y = y ? y : t.gravity;
    }
}