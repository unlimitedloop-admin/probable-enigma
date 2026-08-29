#include "pch.h"

#include "DashingState.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/Probes.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "config/SystemConfig.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus DashingState::Id() const noexcept
    {
        return AvatarStatus::Dashing;
    }

    void DashingState::OnEnter(PlayerContext& cx, StateProvider*, const PlayerTuning& t)
    {
        _elapsed_frames = 0;
        _direction = cx.facingLR;
        cx.animeStepper.reset();
        setPose_(cx, t);
    }

    AvatarStatus DashingState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double)
    {
        using namespace abilities;

        const bool has_direction_input =
            in->IsPressed(JPBTN::LEFT) ^ in->IsPressed(JPBTN::RIGHT);
        const int input_direction = has_direction_input
            ? (in->IsPressed(JPBTN::LEFT) ? -1 : +1)
            : 0;
        const bool reversing = has_direction_input &&
            input_direction != static_cast<int>(_direction);

        if (reversing && hasStandingClearance_(cx, t, 0.0))
        {
            cx.vel = {};
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::StandingA);
            return AvatarStatus::Standing;
        }

        if (reversing)
        {
            _direction = static_cast<AvatarDirection>(input_direction);
        }
        cx.facingLR = _direction;

        cx.probes = makeDashProbes_(cx, t);
        const int direction = static_cast<int>(cx.facingLR);
        const double speed = isStartup_(t) ? t.dashStartSpeed : t.dashSpeed;
        const double requested_dx = speed * static_cast<double>(direction);
        const auto h_hit = cx.terrain->SweepHorizontal(cx.probes, requested_dx);
        cx.vel.x = h_hit.hit ? h_hit.maxDistanceX : requested_dx;
        try_request_horizontal_fixed_scroll(cx, cx.vel.x);

        adjust_vertical_speed_for_gravity(cx, t);
        const auto v_hit = cx.terrain->SweepVertical(cx.probes, cx.vel);
        if (v_hit.hit)
        {
            cx.vel.y = v_hit.maxDistanceY;
            cx.onGround = (v_hit.kind == systems::physics::VHitKind::Floor);
        }
        else
        {
            cx.onGround = false;
        }
        cx.justLanded = (!cx.prevOnGround && cx.onGround);

        setPose_(cx, t);

        if (!cx.onGround)
        {
            cx.vel.x = t.airStrafeVelocity * static_cast<double>(direction);
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::Airpause);
            return AvatarStatus::Hovering;
        }

        const bool standing_clear = hasStandingClearance_(cx, t, cx.vel.x);
        if (cx.jumpEdge && standing_clear)
        {
            cx.probes.refreshAll(cx, t.probeOffsets);
            if (do_jump(cx, t))
            {
                cx.vel.x = t.dashJumpSpeed * static_cast<double>(direction);
                cx.basePose = static_cast<int>(STile::Airpause);
                return AvatarStatus::Hovering;
            }
        }

        if (h_hit.hit && standing_clear)
        {
            cx.vel = {};
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::StandingA);
            return AvatarStatus::Standing;
        }

        if (_elapsed_frames < t.dashFrames)
        {
            ++_elapsed_frames;
        }

        if (_elapsed_frames >= t.dashFrames && standing_clear)
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::RunningA);
            return AvatarStatus::Running;
        }

        return AvatarStatus::Dashing;
    }

    void DashingState::TickAnimationOnly(AnimeContext& ax, StateProvider*, const PlayerTuning& t, double)
    {
        setPose_(ax, t);
    }

    Probes DashingState::makeDashProbes_(const PlayerContext& cx, const PlayerTuning& t) const
    {
        PlayerProbes offsets = t.probeOffsets;
        const double direction = static_cast<double>(cx.facingLR);

        offsets.frontLineOffsetX = 11.0;
        offsets.rearLineOffsetX = -9.0;
        offsets.verticalTopOffsetY = -6.0;
        offsets.verticalMidOffsetY = -6.0;
        offsets.verticalBtmOffsetY = 8.0;
        offsets.horizonFrontOffsetX = direction * 11.0;
        offsets.horizonMidOffsetX = 0.0;
        offsets.horizonBehindOffsetX = direction * -9.0;

        Probes probes{ cx.probes.half };
        probes.refreshAll(cx, offsets);
        return probes;
    }

    bool DashingState::hasStandingClearance_(const PlayerContext& cx, const PlayerTuning& t, double dx) const
    {
        Probes standing_probes{ cx.probes.half };
        standing_probes.refreshAll(cx, t.probeOffsets);
        return !cx.terrain->SweepVertical(
            standing_probes,
            Vec2{ dx, -config::SystemConfig::kEpsilon }).hit;
    }

    bool DashingState::isStartup_(const PlayerTuning& t) const noexcept
    {
        return _elapsed_frames < t.dashStartFrames;
    }

    void DashingState::setPose_(PlayerContext& cx, const PlayerTuning& t) const noexcept
    {
        cx.basePose = static_cast<int>(isStartup_(t) ? STile::DashStart : STile::Dashing);
    }

    void DashingState::setPose_(AnimeContext& ax, const PlayerTuning& t) const noexcept
    {
        ax.basePose = static_cast<int>(isStartup_(t) ? STile::DashStart : STile::Dashing);
    }
}
