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

        if (const auto next = UpdateGroundState(
            cx, in, t, make_input_move_intent(in, t, Id())))
        {
            return *next;
        }

        // Speed down to brake run when no input.
        if (!in->IsPressed(JPBTN::LEFT) && !in->IsPressed(JPBTN::RIGHT))
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::RunningIntro);
            return AvatarStatus::BrakeRun;
        }

        step_running_anim(cx, t);
        return AvatarStatus::Running;
    }

    void RunningState::TickAnimationOnly(AnimeContext& ax, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;

        step_running_anim(ax, t);
        UpdateFacing(ax, in);
    }
}
