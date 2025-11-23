#include "pch.h"

#include "LadderingState.h"

#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus LadderingState::Id() const noexcept { return AvatarStatus::Ladder; }

    AvatarStatus LadderingState::Update(PlayerContext& cx, const InputSnapshot& in, const PlayerTuning& t, double /*dt*/)
    {
        return AvatarStatus::Ladder;
    }
}