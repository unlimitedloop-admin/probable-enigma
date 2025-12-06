#include "pch.h"

#include "GroundBaseState.h"

#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    void GroundBaseState::GroundPipeline(PlayerContext& cx, StateProvider* in, const PlayerTuning& t)
    {
        using namespace abilities;
        // X-axis ground movement.
        GroundMove(cx, Id(), in, t);
        // Y-axis vertical speed preparation.
        AdjustVerticalSpeedForGravity(cx, t);
        // Update onGround status. check below the player's bounding box.
        const bool tempOnGround = cx.terrain->IsGroundLike(cx.facingLR, cx.probes, cx.vel.y);
        cx.onGround = tempOnGround;
        cx.justLanded = (!cx.prevOnGround && cx.onGround);
    }
}