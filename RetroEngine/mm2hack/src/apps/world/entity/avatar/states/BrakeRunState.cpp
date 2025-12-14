#include "pch.h"

#include "BrakeRunState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus BrakeRunState::Id() const noexcept { return AvatarStatus::BrakeRun; }

    AvatarStatus BrakeRunState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;
        // ApplyGroundMove; AdjustVerticalSpeedForGravity; SweepVertical;
        GroundPipeline(cx, in, t, MakeBrakeRunIntent(cx, t));

        // Call after cx.texture is set; adds facing offset (0 right, 40 left for AvatarAnimation enums).
        auto applyFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
                if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
                FacingDirection(cx, cx.facingLR);   // Set facing direction at 'cx.texture'.
            };

        // Begin running if left/right key is pressed.
        if (in->IsPressed(JPBTN::LEFT) || in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.texture = static_cast<int>(STile::RunningIntro);
            applyFacing();
            return AvatarStatus::LaunchRun;
        }

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

        const bool endBrake = StepBrakeRunAnim(cx, t);
        applyFacing();  // Must be after setting cx.texture at StepBrakeRunAnim().
        return endBrake ? AvatarStatus::Standing : AvatarStatus::BrakeRun;
    }
}