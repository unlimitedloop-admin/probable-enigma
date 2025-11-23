//==============================================================================
// 
//  Project: mm2hack
//  MovementAbilities.h
// 
//  Movement abilities for the avatar entity.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar::abilities
{
    using foundation::math::Vec2;

    inline void GroundMove(Vec2& vel, AvatarStatus status, const InputSnapshot& in, const PlayerTuning& t)
    {
        const bool hasLR = (in.left ^ in.right);
        const int  xMoveSign = in.left ? -1 : in.right ? +1 : 0;
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

        // !return
        vel.x = speedX * static_cast<double>(xMoveSign);
    }

    inline void StartJump(Vec2& vel, double impulse)
    {
        vel.y = impulse;
    }

    inline AvatarStatus DoJump(PlayerContext& cx, const PlayerTuning& t)
    {
        // Add jump impulse.
        StartJump(cx.vel, t.jumpImpulse);
        return AvatarStatus::Hovering;
    }

    inline void AirMove(Vec2& vel, bool left, bool right, double accel, double maxspd)
    {
        if (left ^ right)
        {
            double s = left ? -1.0 : 1.0;
            vel.x += s * accel;
            if (vel.x > maxspd) vel.x = maxspd;
            if (vel.x < -maxspd) vel.x = -maxspd;
        }
        // NOTE: Do not decelerate in air. (Realistic physics)
    }

    inline void ApplyGravity(Vec2& vel, double g, double terminal)
    {
        vel.y += g;
        if (vel.y > terminal) vel.y = terminal;
    }

    inline AvatarStatus DoFalling(PlayerContext& cx, const PlayerTuning& t, bool isJump)
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
        return AvatarStatus::Hovering;
    }
}