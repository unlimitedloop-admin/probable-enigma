#include "pch.h"

#include "HoveringState.h"

#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
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
            const double actualDx = intent.speed * static_cast<double>(intent.dirSign);
            if (actualDx > 0.0)
            {
                FixedScrollRequest req{};
                req.dir = PageScroll::Dir::Right;

                // Calculate edge gap in world px at request time.
                const double frontX = cx.probes.frontLine.middlePoint.x;// world pos.
                const double rightEdge = cx.vBounds.rightX;             // world pos.
                req.edgeGapPx = std::max(0.0, rightEdge - frontX);

                cx.pendingFixedScroll = req;
            }
            else if (actualDx < 0.0)
            {
                FixedScrollRequest req{};
                req.dir = PageScroll::Dir::Left;

                // Calculate edge gap in world px at request time.
                const double frontX = cx.probes.frontLine.middlePoint.x;// world pos.
                const double leftEdge = cx.vBounds.leftX;               // world pos.
                req.edgeGapPx = std::max(0.0, frontX - leftEdge);
                cx.pendingFixedScroll = req;
            }
        }

        // Jump or falling [Yaxis] movement. (Common airborne behavior)
        UpdateVerticalVelocity(cx, t, in->IsPressed(JPBTN::A));

        // Call after cx.texture is set; adds facing offset (0 right, 40 left for AvatarAnimation enums).
        auto applyFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
                if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
                FacingDirection(cx.texture, cx.facingLR);   // Set facing direction at 'cx.texture'.
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
            cx.onGround = false;
        }
        cx.justLanded = (!cx.prevOnGround && cx.onGround);

        if (cx.justLanded)
        {
            if (in->JustPressed(JPBTN::A))
            {
                DoJump(cx, t);
                cx.texture = static_cast<int>(STile::Airpause);
                applyFacing();
                return AvatarStatus::Hovering;
            }
            else if (in->IsPressed(JPBTN::LEFT) || in->IsPressed(JPBTN::RIGHT))
            {
                cx.texture = static_cast<int>(STile::RunningA);
                applyFacing();
                return AvatarStatus::Running;
            }
            else
            {
                LandingAnim(cx, t);
                applyFacing();
                return AvatarStatus::Landing;
            }
        }

        cx.texture = static_cast<int>(STile::Airpause);
        applyFacing();
        return AvatarStatus::Hovering;
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
}