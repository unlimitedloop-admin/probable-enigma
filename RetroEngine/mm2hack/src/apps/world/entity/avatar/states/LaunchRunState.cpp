#include "pch.h"

#include "LaunchRunState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus LaunchRunState::Id() const noexcept { return AvatarStatus::LaunchRun; }

    AvatarStatus LaunchRunState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;

        // Branch to laddering state if ladder is detected.
        if (TryEnterLadderFromGround(cx, in))
        {
            //cx.animeStepper.reset();    // DELETE: This is done by the LadderingState::OnEnter().
            return AvatarStatus::Laddering;
        }
        // ApplyGroundMove; AdjustVerticalSpeedForGravity; SweepVertical;
        GroundPipeline(cx, in, t, MakeInputMoveIntent(in, t, Id()));

        // Call after cx.basePose is set; adds facing offset (0 right, 40 left for AvatarAnimation enums).
        auto updateFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
                if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
            };

        if (!cx.onGround)
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::Airpause);
            updateFacing();
            return AvatarStatus::Hovering;
        }

        if (in->JustPressed(JPBTN::A) && DoJump(cx, t))
        {
            cx.basePose = static_cast<int>(STile::Airpause);
            updateFacing();
            return AvatarStatus::Hovering;
        }

        // Back to standing if no input.
        if (!in->IsPressed(JPBTN::LEFT) && !in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::StandingA);
            updateFacing();
            return AvatarStatus::Standing;
        }

        const bool bursted = StepLaunchRunAnim(cx, t);
        updateFacing();  // Must be after setting cx.basePose at StepLaunchRunAnim().
        return bursted ? AvatarStatus::Running : AvatarStatus::LaunchRun;
    }
}