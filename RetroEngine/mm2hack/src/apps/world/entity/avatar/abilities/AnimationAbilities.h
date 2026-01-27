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
    inline AvatarDirection OppositeFacingDirection(const AvatarDirection dir) noexcept
    {
        return (dir == AvatarDirection::Left) ? AvatarDirection::Right : AvatarDirection::Left;
    }

    // Returns true when the launch run animation has completed and switched to running
    inline bool StepLaunchRunAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // When RunningIntro animation between cycles 0.
        if (cx.animeStepper.tick < t.reactionFrameRun)
        {
            ++cx.animeStepper.tick;
            cx.basePose = static_cast<int>(AvatarAnimation::RunningIntro);
            return false;
        }
        // After that, switch to RunningA animation (When holding pressing the right/Left arrow key).
        cx.animeStepper.reset();
        cx.basePose = static_cast<int>(AvatarAnimation::RunningA);
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
        cx.basePose = static_cast<int>(runningTextures[cx.animeStepper.frame]);
    }

    // Steps the running animation (animation-only context)
    inline void StepRunningAnim(AnimeContext& ax, const PlayerTuning& t)
    {
        // Step the running animation (A, C, B, C ... and loop).
        static const std::array<STile, 4> runningTextures = {
            STile::RunningA, STile::RunningC, STile::RunningB, STile::RunningC
        };
        ax.animeStepper.step(7, static_cast<int>(runningTextures.size()), 1); // 7 ticks per frame, 4 frames.
        ax.basePose = static_cast<int>(runningTextures[ax.animeStepper.frame]);
    }

    // Returns true if the brake is fully engaged
    inline bool StepBrakeRunAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // Step the brake run animation.
        const bool committed = cx.animeStepper.step(4, 2); // 4 ticks per frame, 2 frames.
        if (!committed)
        {
            cx.basePose = static_cast<int>(STile::RunningIntro);
            return false;
        }
        // After that, switch to StandingA animation.
        cx.basePose = static_cast<int>(STile::StandingA);
        return true;
    }

    // Returns true if the brake is fully engaged
    inline bool LandingAnim(PlayerContext& cx, const PlayerTuning& t)
    {
        // Step the landing animation.
        const bool committed = cx.animeStepper.step(2, 2); // 2 ticks per frame, 2 frames.
        if (!committed)
        {
            cx.basePose = static_cast<int>(STile::RunningB);
            return false;
        }
        // After that, switch to StandingA animation.
        cx.basePose = static_cast<int>(STile::StandingA);
        return true;
    }

    // Laddering animation
    inline void LadderingAnim(PlayerContext& cx, const int input, const bool isTopAttrEmpty)
    {
        // Simple climbing animation between LadderingA and LadderingB.
        if (!input)
        {
            // Reset animation when no vertical input.
            cx.animeStepper.resetTick();
        }
        else
        {
            cx.animeStepper.step(9, 2); // 9 ticks per frame, 2 frames.
        }

        if (cx.animeStepper.frame < 1)
        {
            // input == -1 is climbing up, Change to a specified sprite tile near the top of the ladder.
            cx.basePose = isTopAttrEmpty && (input == -1) ? static_cast<int>(STile::LadderTopA) : static_cast<int>(STile::LadderingA);
        }
        else
        {
            // input == -1 is climbing up, Change to a specified sprite tile near the top of the ladder.
            cx.basePose = isTopAttrEmpty && (input == -1) ? static_cast<int>(STile::LadderTopB) : static_cast<int>(STile::LadderingB);
        }
    }

    inline void LadderingAnim(AnimeContext& ax)
    {
        ax.animeStepper.step(9, 2); // 9 ticks per frame, 2 frames.

        if (ax.animeStepper.frame < 1)
        {
            // input == -1 is climbing up, Change to a specified sprite tile near the top of the ladder.
            ax.basePose = static_cast<int>(STile::LadderingA);
        }
        else
        {
            // input == -1 is climbing up, Change to a specified sprite tile near the top of the ladder.
            ax.basePose = static_cast<int>(STile::LadderingB);
        }
    }

    inline void WaitingAnim(PlayerContext& cx)
    {
        // Simple waiting animation between StandingA and StandingB.
        cx.animeStepper.step(9, 11, 1); // 9 ticks per frame, 11 frames.
        if (cx.animeStepper.frame < 10)
        {
            cx.basePose = static_cast<int>(STile::StandingA);
        }
        else
        {
            cx.basePose = static_cast<int>(STile::StandingB);
        }
    }

    inline void ConstantAnim(AnimeContext& ax)
    {

    }
}