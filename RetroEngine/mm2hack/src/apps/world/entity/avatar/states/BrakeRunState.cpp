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

        if (const auto next = UpdateGroundState(
            cx, in, t, make_brake_run_intent(cx, t)))
        {
            return *next;
        }

        // Begin running if left/right key is pressed.
        if (in->IsPressed(JPBTN::LEFT) || in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::RunningIntro);
            return AvatarStatus::LaunchRun;
        }

        const bool endBrake = step_brake_run_anim(cx, t);
        return endBrake ? AvatarStatus::Standing : AvatarStatus::BrakeRun;
    }
}
