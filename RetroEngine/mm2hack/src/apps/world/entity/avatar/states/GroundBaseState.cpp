#include "pch.h"

#include "GroundBaseState.h"

#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    void GroundBaseState::GroundPipeline(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, const GroundMoveIntent intent)
    {
        using namespace abilities;
        // X-axis ground movement.
        ApplyGroundMove(cx, intent);
        // Y-axis vertical speed preparation.
        AdjustVerticalSpeedForGravity(cx, t);
        // Update onGround status. check below the player's bounding box.
        auto hit = cx.terrain->SweepVertical(cx.probes, cx.vel.y);
        if (hit.hit)
        {
            cx.vel.y = hit.maxDistanceY;
            cx.onGround = true;
        }
        else
        {
            cx.onGround = false;
        }
        cx.justLanded = (!cx.prevOnGround && cx.onGround);
    }
}