#include "pch.h"

#include "HoveringState.h"

#include "apps/systems/physics/ITerrainProbe.h"
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
        // X-axis air movement.
        auto intent = MakeAirMoveIntent(in, t);

        if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
        if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
        cx.probes.swapFrontLR(cx, t.probeOffsets); // Update front/rear probes based on facing direction.

        ApplyAirControl(cx, intent);
        ApplyAirMove(cx, intent);

        // Jump or falling [Yaxis] movement. (Common airborne behavior)
        UpdateVerticalVelocity(cx, t, in->IsPressed(JPBTN::A));

        // Call after cx.texture is set; adds facing offset (0 right, 40 left for AvatarAnimation enums).
        auto applyFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
                if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
                FacingDirection(cx, cx.facingLR);   // Set facing direction at 'cx.texture'.
            };

        // Y-axis air movement.
        // Update onGround status. check below the player's bounding box.
        auto hit = cx.terrain->SweepVertical(cx.probes, cx.vel);
        if (hit.hit)
        {
            cx.vel.y = hit.maxDistanceY;
            cx.onGround = (hit.kind == systems::physics::VHitKind::Floor);
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
                //LandingMove(cx, t);
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
}