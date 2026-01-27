#include "pch.h"

#include "RunningState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus RunningState::Id() const noexcept { return AvatarStatus::Running; }

    AvatarStatus RunningState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
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

        // Speed down to brake run when no input.
        if (!in->IsPressed(JPBTN::LEFT) && !in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::RunningIntro);
            updateFacing();
            return AvatarStatus::BrakeRun;
        }

        StepRunningAnim(cx, t);
        updateFacing();  // Must be after setting cx.basePose at StepRunningAnim().
        return AvatarStatus::Running;
    }

    void RunningState::TickAnimationOnly(AnimeContext& ax, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;

        // Call after cx.basePose is set; adds facing offset (0 right, 40 left for AvatarAnimation enums).
        auto updateFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))  ax.facingLR = AvatarDirection::Left;
                if (in->IsPressed(JPBTN::RIGHT)) ax.facingLR = AvatarDirection::Right;
            };

        StepRunningAnim(ax, t);
        updateFacing();  // Must be after setting cx.basePose at StepRunningAnim().
    }
}