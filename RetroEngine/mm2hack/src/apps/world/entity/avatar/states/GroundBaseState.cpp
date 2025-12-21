#include "pch.h"

#include "GroundBaseState.h"

#include <cstdlib>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ILadderService.h"
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

    bool GroundBaseState::TryEnterLadderFromGround(PlayerContext& cx, StateProvider* in) const
    {
        if (cx.ladder == nullptr)
        {
            return false;
        }

        const bool up = in->IsPressed(JPBTN::UP);
        const bool down = in->IsPressed(JPBTN::DOWN);
        if (!up && !down)
        {
            return false;
        }

        // Up: any behind probe point overlaps ladder.
        if (up)
        {
            const auto& b = cx.probes.behindGround;
            if (cx.ladder->CanGrabAt(b.topPoint) || cx.ladder->CanGrabAt(b.middlePoint) || cx.ladder->CanGrabAt(b.bottomPoint))
            {
                cx.ladder->setEntryKind(systems::physics::LadderEntryKind::NormalGrab);
                return true;
            }
        }

        // Down: when descending from the ground, check the 3 points of bottomLine's y + 0x00.01p0
        if (down)
        {
            const auto& bl = cx.probes.bottomLine; // Assumes it has left/middle/right
            const Vec2 p{ bl.middlePoint.x, bl.middlePoint.y + config::SystemConfig::kEpsilon };

            if (cx.ladder->CanGrabAt(p))
            {
                cx.ladder->setEntryKind(systems::physics::LadderEntryKind::FromTopDown);
                return true;
            }
        }

        return false;
    }
}