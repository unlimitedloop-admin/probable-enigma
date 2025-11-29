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

        if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
        if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
        FacingDirection(cx, cx.facingLR);   // Set facing direction.
        GroundMove(cx.vel, Id(), in, t);    // X-axis ground movement.

        // Begin running if left/right key is pressed.
        if (in->IsPressed(JPBTN::LEFT) || in->IsPressed(JPBTN::RIGHT))
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