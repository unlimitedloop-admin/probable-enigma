#include "pch.h"

#include "StandingState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus StandingState::Id() const noexcept { return AvatarStatus::Standing; }

    AvatarStatus StandingState::Update(PlayerContext& cx, const InputSnapshot& in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;

        if (in.left)  cx.facingLR = AvatarDirection::Left;
        if (in.right) cx.facingLR = AvatarDirection::Right;
        FacingDirection(cx, cx.facingLR);   // Set facing direction.
        GroundMove(cx.vel, Id(), in, t);    // X-axis ground movement.

        // Begin running if left/right key is pressed.
        if (in.right || in.left)
        {
            cx.animeStepper.reset();
            cx.texture = static_cast<int>(STile::RunningIntro);
            return AvatarStatus::LaunchRun;
        }

        cx.animeStepper.reset();
        cx.texture = static_cast<int>(STile::StandingA);
        return AvatarStatus::Standing;
    }
}