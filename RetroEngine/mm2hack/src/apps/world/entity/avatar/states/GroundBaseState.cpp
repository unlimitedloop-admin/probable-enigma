#include "pch.h"

#include "GroundBaseState.h"

#include <cstdlib>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
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
        using namespace systems::scrolling::atomic;
        using PageDir = PageScroll::Dir;
        using PageGridIndex = systems::physics::PageGridIndex;

        // Update facing.
        if (in->IsPressed(JPBTN::LEFT)) { cx.facingLR = AvatarDirection::Left; }
        if (in->IsPressed(JPBTN::RIGHT)) { cx.facingLR = AvatarDirection::Right; }

        // Update front/rear probes based on facing direction.
        cx.probes.swapFrontLR(cx, t.probeOffsets);

        // X-axis ground movement. Check horizontal collisions.
        const double dx = intent.speed * static_cast<double>(intent.dirSign);

        // Save the front-probe X BEFORE movement for boundary-cross detection (world space). (Unused?)
        //const double prevFrontX = cx.probes.frontLine.middlePoint.x;

        auto hHit = cx.terrain->SweepHorizontal(cx.probes, dx);
        if (hHit.hit)
        {
            intent.speed = std::abs(hHit.maxDistanceX);
        }

        // Apply velocity based on updated intent.
        ApplyGroundMove(cx, intent);

        // ---- Fixed page scroll request by boundary crossing (NOT by hit) ----
        // We base this on the movement that will actually happen this frame.
        if (intent.active)
        {
            constexpr double kTriggerGapPx = 14.0;

            // actual movement direction for this frame
            const double actualDx = intent.speed * static_cast<double>(intent.dirSign);

            if (actualDx > 0.0)
            {
                // world pos.s
                const double frontX = cx.probes.frontLine.middlePoint.x;
                const double rightEdge = cx.vBounds.rightX;

                // only when the probe reaches near the edge
                if ((rightEdge - frontX) <= kTriggerGapPx)
                {
                    // only if the neighbor is FixedPage
                    if (cx.scrollRules && IsFixedScroll(cx.scrollRules->RightType(cx.scrollPageIndex)))
                    {
                        FixedScrollRequest req{};
                        req.dir = PageScroll::Dir::Right;
                        req.carryTotalPx = 48.0;
                        cx.pendingFixedScroll = req;
                    }
                }
            }
            else if (actualDx < 0.0)
            {
                // world pos.
                const double frontX = cx.probes.frontLine.middlePoint.x;
                const double leftEdge = cx.vBounds.leftX;

                if ((frontX - leftEdge) <= kTriggerGapPx)
                {
                    if (cx.scrollRules && IsFixedScroll(cx.scrollRules->LeftType(cx.scrollPageIndex)))
                    {
                        FixedScrollRequest req{};
                        req.dir = PageScroll::Dir::Left;
                        req.carryTotalPx = 48.0;
                        cx.pendingFixedScroll = req;
                    }
                }
            }
        }

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