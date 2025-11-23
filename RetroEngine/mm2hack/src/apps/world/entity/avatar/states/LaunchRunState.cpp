#include "pch.h"

#include "LaunchRunState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus LaunchRunState::Id() const noexcept { return AvatarStatus::LaunchRun; }

    AvatarStatus LaunchRunState::Update(PlayerContext& cx, const InputSnapshot& in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;

        if (in.left)  cx.facingLR = AvatarDirection::Left;
        if (in.right) cx.facingLR = AvatarDirection::Right;
        FacingDirection(cx, cx.facingLR);   // Set facing direction.
        GroundMove(cx.vel, Id(), in, t);    // X-axis ground movement.

        // Back to standing if no input.
        if (!in.left && !in.right)
        {
            cx.animeStepper.reset();
            cx.texture = static_cast<int>(STile::StandingA);
            return AvatarStatus::Standing;
        }

        const bool bursted = StepLaunchRunAnim(cx, t);
        return bursted ? AvatarStatus::Running : AvatarStatus::LaunchRun;
    }
}