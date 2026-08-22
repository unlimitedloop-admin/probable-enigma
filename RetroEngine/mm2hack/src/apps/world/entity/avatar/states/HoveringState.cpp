#include "pch.h"

#include "HoveringState.h"

#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus HoveringState::Id() const noexcept { return AvatarStatus::Hovering; }

    AvatarStatus HoveringState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;
        using namespace systems::scrolling::atomic;
        using PageDir = PageScroll::Dir;
        using PageGridIndex = systems::physics::PageGridIndex;

        // Branch to laddering state if ladder is detected.
        if (tryEnterLadder_(cx, in, t))
        {
            //cx.animeStepper.reset();    // DELETE: This is done by the LadderingState::OnEnter().
            return AvatarStatus::Laddering;
        }
        // X-axis air movement.
        auto intent = MakeAirMoveIntent(in, t);

        if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
        if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
        cx.probes.swapFrontLR(cx, t.probeOffsets); // Update front/rear probes based on facing direction.

        ApplyAirControl(cx, intent);
        ApplyAirMove(cx, intent);

        // ---- Fixed page scroll request by boundary crossing (NOT by hit) ----
        // We base this on the movement that will actually happen this frame.
        if (intent.active)
        {
            constexpr double kTriggerGapPx = 14.0;
            const double actualDx = intent.speed * intent.dirSign;
            if (cx.pendingFixedScroll.available && actualDx > 0.0)
            {
                const double frontX = cx.probes.frontLine.middlePoint.x;
                const double rightEdge = cx.vBounds.rightX;

                if (rightEdge - frontX <= kTriggerGapPx)
                {
                    cx.pendingFixedScroll = { false, PageScroll::Dir::Right, 48.0 };
                }
            }
            else if (cx.pendingFixedScroll.available && actualDx < 0.0)
            {
                const double frontX = cx.probes.frontLine.middlePoint.x;
                const double leftEdge = cx.vBounds.leftX;

                if (frontX - leftEdge <= kTriggerGapPx)
                {
                    cx.pendingFixedScroll = { false, PageScroll::Dir::Left, 48.0 };
                }
            }
        }

        // Jump or falling [Yaxis] movement. (Common airborne behavior)
        UpdateVerticalVelocity(cx, t, in->IsPressed(JPBTN::A));

        // Call after cx.basePose is set; adds facing offset (0 right, 40 left for AvatarAnimation enums).
        auto updateFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
                if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
            };

        // Y-axis air movement.
        // Update onGround status. check below the player's bounding box.
        double origVelY = cx.vel.y;
        auto hit = cx.terrain->SweepVertical(cx.probes, cx.vel);
        if (hit.hit)
        {
            cx.vel.y = hit.maxDistanceY;
            cx.onGround = (hit.kind == systems::physics::VHitKind::Floor);
            cx.isHitCeiling = (hit.kind == systems::physics::VHitKind::Ceiling);
        }
        else
        {
            fixedScrollingY_(cx);
            cx.onGround = false;
        }
        cx.justLanded = (!cx.prevOnGround && cx.onGround);

        if (cx.justLanded)
        {
            cx.output.PushEvent(PlayerEventType::Landed);

            if (cx.jumpEdge)
            {
                DoJump(cx, t);
                cx.basePose = static_cast<int>(STile::Airpause);
                updateFacing();
                return AvatarStatus::Hovering;
            }
            else if (in->IsPressed(JPBTN::LEFT) || in->IsPressed(JPBTN::RIGHT))
            {
                cx.basePose = static_cast<int>(STile::RunningA);
                updateFacing();
                return AvatarStatus::Running;
            }
            else
            {
                LandingAnim(cx, t);
                updateFacing();
                return AvatarStatus::Landing;
            }
        }

        cx.basePose = static_cast<int>(STile::Airpause);
        updateFacing();
        return AvatarStatus::Hovering;
    }

    bool HoveringState::tryEnterLadder_(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) const
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

        const auto& b = cx.probes.behindGround;
        if (cx.ladder->CanGrabAt(b.topPoint) || cx.ladder->CanGrabAt(b.middlePoint) || cx.ladder->CanGrabAt(b.bottomPoint))
        {
            cx.ladder->setEntryKind(systems::physics::LadderEntryKind::NormalGrab);
            return true;
        }

        return false;
    }

    // Resolve vertical collision when a hit is reported by SweepVertical.
    void HoveringState::resolveVerticalCollision_(PlayerContext& cx, const PlayerTuning& t, double origVelY, const apps::systems::physics::SweepVHit& hit) noexcept
    {
        cx.vel.y = hit.maxDistanceY;

        // If hit the ceiling and "moving up (jumping)", replace with specified speed.
        if (hit.kind == systems::physics::VHitKind::Ceiling && origVelY < 0.0)
        {
            cx.vel.y = 0.0;
        }

        // onGround is only for floor detection.
        cx.onGround = (hit.kind == systems::physics::VHitKind::Floor);
    }

    void HoveringState::fixedScrollingY_(PlayerContext& cx) const noexcept
    {
        using conf = config::SystemConfig;
        using namespace systems::scrolling::atomic;

        const int page_h = conf::kTileCountY * conf::kTileSize;
        const double actualDy = cx.vel.y;
        
        // Falling down only.
        if (actualDy > 0.0)
        {
            // World Y of the probes before and after movement.
            const double probePrevWorldY = cx.prelimProbes.behindGround.middlePoint.y;
            const double probeCurrWorldY = cx.probes.behindGround.middlePoint.y + actualDy;

            const auto origin = cx.pageOriginPx;
            const double prevLocalY = probePrevWorldY - origin.y;
            const double currLocalY = probeCurrWorldY - origin.y;

            if (prevLocalY < static_cast<double>(page_h) && currLocalY >= static_cast<double>(page_h))
            {
                FixedScrollRequest req{};
                req.dir = PageScroll::Dir::Down;
                req.carryTotalPx = 0x01.00p0;
                cx.pendingFixedScroll = req;
            }
        }
        else
        {
            // NOTE: Cannot scroll up when moving up in Hovering state.
        }
    }
}
