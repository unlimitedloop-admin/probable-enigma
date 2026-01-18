#include "pch.h"

#include "StandingState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus StandingState::Id() const noexcept { return AvatarStatus::Standing; }

    AvatarStatus StandingState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
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

        // Call after cx.texture is set; adds facing offset (0 right, 40 left for AvatarAnimation enums).
        auto applyFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
                if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
                FacingDirection(cx.texture, cx.facingLR);   // Set facing direction at 'cx.texture'.
            };

        if (!cx.onGround)
        {
            cx.animeStepper.reset();
            cx.texture = static_cast<int>(STile::Airpause);
            applyFacing();
            return AvatarStatus::Hovering;
        }

        if (in->JustPressed(JPBTN::A) && DoJump(cx, t))
        {
            cx.texture = static_cast<int>(STile::Airpause);
            applyFacing();
            return AvatarStatus::Hovering;
        }

        // Begin running if left/right key is pressed.
        if (in->IsPressed(JPBTN::LEFT) || in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.texture = static_cast<int>(STile::RunningIntro);
            applyFacing();
            return AvatarStatus::LaunchRun;
        }

        WaitingAnim(cx);
        applyFacing();
        return AvatarStatus::Standing;
    }
}