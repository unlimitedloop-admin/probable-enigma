#include "pch.h"

#include "GroundBaseState.h"

#include <cstdlib>
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    void GroundBaseState::GroundPipeline(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, GroundMoveIntent intent)
    {
        using namespace abilities;

        if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
        if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
        cx.probes.swapFrontLR(cx, t.probeOffsets); // Update front/rear probes based on facing direction.

        // X-axis ground movement. check horizontal collisions.
        const double dx = intent.speed * static_cast<double>(intent.dirSign);
        auto hHit = cx.terrain->SweepHorizontal(cx.probes, dx);
        if (hHit.hit)
        {
            intent.speed = std::abs(hHit.maxDistanceX);
        }
        ApplyGroundMove(cx, intent);

        // Y-axis vertical speed preparation.
        AdjustVerticalSpeedForGravity(cx, t);
        
        // Update onGround status. check vertical collisions.
        auto vHit = cx.terrain->SweepVertical(cx.probes, cx.vel);
        if (vHit.hit)
        {
            cx.vel.y = vHit.maxDistanceY;
            cx.onGround = (vHit.kind == systems::physics::VHitKind::Floor);
        }
        else
        {
            cx.onGround = false;
        }
        cx.justLanded = (!cx.prevOnGround && cx.onGround);
    }
}