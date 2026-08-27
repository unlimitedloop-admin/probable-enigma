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

        if (TryEnterSliding(cx, in))
        {
            return AvatarStatus::Sliding;
        }

        // Branch to laddering state if ladder is detected.
        if (TryEnterLadderFromGround(cx, in))
        {
            return AvatarStatus::Laddering;
        }
        // apply_ground_move; adjust_vertical_speed_for_gravity; SweepVertical;
        GroundPipeline(cx, in, t, make_brake_run_intent(cx, t));

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

        if (cx.jumpEdge && do_jump(cx, t))
        {
            cx.basePose = static_cast<int>(STile::Airpause);
            updateFacing();
            return AvatarStatus::Hovering;
        }

        // Begin running if left/right key is pressed.
        if (in->IsPressed(JPBTN::LEFT) || in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::RunningIntro);
            updateFacing();
            return AvatarStatus::LaunchRun;
        }

        const bool endBrake = step_brake_run_anim(cx, t);
        updateFacing();  // Must be after setting cx.basePose at step_brake_run_anim().
        return endBrake ? AvatarStatus::Standing : AvatarStatus::BrakeRun;
    }
}
