#include "pch.h"

#include "LandingState.h"

#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus LandingState::Id() const noexcept { return AvatarStatus::Landing; }

    AvatarStatus LandingState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;

        if (const auto next = UpdateGroundState(
            cx, in, t, make_input_move_intent(in, t, Id())))
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

        // Execute landing animation if not under control.
        const bool landed = landing_anim(cx, t);
        return landed ? AvatarStatus::Standing : AvatarStatus::Landing;
    }
}
