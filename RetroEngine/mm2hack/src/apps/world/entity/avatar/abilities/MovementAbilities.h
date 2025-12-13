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

    // *HorizontalGroundMove; sets horizontal speed based on status and input (left/right)
    inline void GroundMove(PlayerContext& cx, AvatarStatus status, StateProvider* in, const PlayerTuning& t)
    {
        const bool leftPressed  = in->IsPressed(JPBTN::LEFT);
        const bool rightPressed = in->IsPressed(JPBTN::RIGHT);
        const bool hasLR        = (leftPressed ^ rightPressed);

        int xMoveSign = leftPressed ? -1 : rightPressed ? +1 : 0;
        double speedX = 0.0;

        if (hasLR)
        {
            switch (status)
            {
            case AvatarStatus::Standing:
                // NOTE: Standing but can only be reached when the left or right key is pressed.
                speedX = t.momentumStart;
                break;
            case AvatarStatus::LaunchRun:
                speedX = t.momentumStart;
                break;
            case AvatarStatus::Running:
                speedX = t.steadyRun;
                break;
            case AvatarStatus::BrakeRun:
                speedX = t.haltSpeed;
                xMoveSign = cx.facingLR == AvatarDirection::Left ? -1 : 1;  // NOTE: BrakeRun always moves toward facing direction.
                break;
            default:
                speedX = 0.0;
                break;
            }
        }
        else
        {
            // Stopped.
            speedX = 0.0;
        }

        // !Effectively the return value.
        cx.vel.x = speedX * static_cast<double>(xMoveSign);
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
        if (cx.terrain->SweepVertical(cx.probes, 1.0).hit)
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
            vel.x += s * accel;
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

        double y = 0.0, f = 0.0;
        y = -std::modf(cx.pos.y, &f);
        cx.vel.y = y ? y : t.gravity;
    }
}