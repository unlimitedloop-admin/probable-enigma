#include "pch.h"

#include "HoveringState.h"

#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus HoveringState::Id() const noexcept { return AvatarStatus::Hovering; }

    AvatarStatus HoveringState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        return AvatarStatus::Hovering;
    }
}