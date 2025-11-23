#include "pch.h"

#include "RunningState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus RunningState::Id() const noexcept { return AvatarStatus::Running; }

    AvatarStatus RunningState::Update(PlayerContext& cx, const InputSnapshot& in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;

        if (in.left)  cx.facingLR = AvatarDirection::Left;
        if (in.right) cx.facingLR = AvatarDirection::Right;
        FacingDirection(cx, cx.facingLR);   // Set facing direction.
        GroundMove(cx.vel, Id(), in, t);    // X-axis ground movement.

        // Speed down to brake run when no input.
        if (!in.left && !in.right)
        {
            cx.animeStepper.reset();
            cx.texture = static_cast<int>(STile::RunningIntro);
            return AvatarStatus::BrakeRun;
        }

        StepRunningAnim(cx, t);
        return AvatarStatus::Running;
    }
}