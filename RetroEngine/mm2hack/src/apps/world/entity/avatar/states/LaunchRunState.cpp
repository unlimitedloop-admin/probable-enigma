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

        if (const auto next = UpdateGroundState(
            cx, in, t, make_input_move_intent(in, t, Id())))
        {
            return *next;
        }

        // Back to standing if no input.
        if (!in->IsPressed(JPBTN::LEFT) && !in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::StandingA);
            return AvatarStatus::Standing;
        }

        const bool bursted = step_launch_run_anim(cx, t);
        return bursted ? AvatarStatus::Running : AvatarStatus::LaunchRun;
    }
}
