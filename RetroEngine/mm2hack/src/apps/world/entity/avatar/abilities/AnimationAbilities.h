//==============================================================================
// 
//  Project: mm2hack
//  AnimationAbilities.h
// 
//  Animation abilities for the avatar.
// 
//==============================================================================
#pragma once

#include <array>
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar::abilities
{
    // Avatar facing direction enumeration
    inline bool FacingDirection(PlayerContext& cx, const AvatarDirection facingLR)
    {
        // This function can be expanded to set the avatar's facing direction
        // based on the facingLR value (-1 for left, +1 for right).
        // Currently, it's a placeholder for potential future use.
        switch (facingLR)
        {
        case AvatarDirection::Left:
            // Set the avatar's facing direction to left.
            cx.texture = cx.texture + static_cast<int>(AvatarAnimation::ToTheLeft);
            return true;
        case AvatarDirection::Right:
            // Set the avatar's facing direction to right.
            cx.texture = cx.texture + static_cast<int>(AvatarAnimation::ToTheRight);
            return true;
        default:
            return false;
        }
    }

    // Returns true when the launch run animation has completed and switched to running
    inline bool StepLaunchRunAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // When RunningIntro animation between cycles 0.
        if (cx.animeStepper.tick < t.reactionFrameRun)
        {
            ++cx.animeStepper.tick;
            cx.texture = static_cast<int>(AvatarAnimation::RunningIntro);
            return false;
        }
        // After that, switch to RunningA animation (When holding pressing the right/Left arrow key).
        cx.animeStepper.reset();
        cx.texture = static_cast<int>(AvatarAnimation::RunningA);
        return true;
    }

    // Steps the running animation
    inline void StepRunningAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // Step the running animation (A, C, B, C ... and loop).
        static const std::array<STile, 4> runningTextures = {
            STile::RunningA, STile::RunningC, STile::RunningB, STile::RunningC
        };
        cx.animeStepper.step(7, static_cast<int>(runningTextures.size()), 1); // 7 ticks per frame, 4 frames.
        cx.texture = static_cast<int>(runningTextures[cx.animeStepper.frame]);
    }

    // Returns true if the brake is fully engaged
    inline bool StepBrakeRunAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // Step the brake run animation.
        const bool committed = cx.animeStepper.step(4, 2); // 4 ticks per frame, 2 frames.
        if (!committed)
        {
            cx.texture = static_cast<int>(STile::RunningIntro);
            return false;
        }
        // After that, switch to StandingA animation.
        cx.texture = static_cast<int>(STile::StandingA);
        return true;
    }

    // Returns true if the brake is fully engaged
    inline bool LandingAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // Step the landing animation.
        const bool committed = cx.animeStepper.step(2, 2); // 2 ticks per frame, 2 frames.
        if (!committed)
        {
            return false;
        }
        // After that, switch to StandingA animation.
        cx.texture = static_cast<int>(STile::StandingA);
        return true;
    }

    inline void WaitingAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // Simple waiting animation between StandingA and StandingB.
        cx.animeStepper.step(9, 11, 1); // 9 ticks per frame, 11 frames.
        if (cx.animeStepper.frame < 10)
        {
            cx.texture = static_cast<int>(STile::StandingA);
        }
        else
        {
            cx.texture = static_cast<int>(STile::StandingB);
        }
    }
}