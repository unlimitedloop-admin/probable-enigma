#include "pch.h"

#include "LandingState.h"

#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus LandingState::Id() const noexcept { return AvatarStatus::Landing; }

    AvatarStatus LandingState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        return AvatarStatus::Landing;
    }
}