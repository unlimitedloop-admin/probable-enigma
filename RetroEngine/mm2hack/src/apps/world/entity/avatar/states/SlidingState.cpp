#include "pch.h"

#include "SlidingState.h"

#include <cmath>
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/Probes.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "config/SystemConfig.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    AvatarStatus SlidingState::Id() const noexcept
    {
        return AvatarStatus::Sliding;
    }

    void SlidingState::OnEnter(PlayerContext& cx, StateProvider*, const PlayerTuning&)
    {
        _elapsed_frames = 0;
        cx.animeStepper.reset();
        setPose_(cx);
    }

    AvatarStatus SlidingState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double)
    {
        using namespace abilities;

        const bool has_direction_input =
            in->IsPressed(JPBTN::LEFT) ^ in->IsPressed(JPBTN::RIGHT);
        const int input_direction = has_direction_input
            ? (in->IsPressed(JPBTN::LEFT) ? -1 : +1)
            : 0;
        const bool reversing = has_direction_input &&
            input_direction != static_cast<int>(cx.facingLR);

        if (reversing && hasStandingClearance_(cx, t, 0.0))
        {
            cx.vel = {};
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::StandingA);
            return AvatarStatus::Standing;
        }

        if (has_direction_input)
        {
            cx.facingLR = static_cast<AvatarDirection>(input_direction);
        }

        cx.probes = makeSlidingProbes_(cx, t);
        const int direction = static_cast<int>(cx.facingLR);
        const double requested_dx = t.slidingSpeed * static_cast<double>(direction);
        const auto h_hit = cx.terrain->SweepHorizontal(cx.probes, requested_dx);
        cx.vel.x = h_hit.hit ? h_hit.maxDistanceX : requested_dx;
        TryRequestHorizontalFixedScroll(cx, cx.vel.x);

        AdjustVerticalSpeedForGravity(cx, t);
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

        setPose_(cx);

        if (!cx.onGround)
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::Airpause);
            return AvatarStatus::Hovering;
        }

        const bool standing_clear = hasStandingClearance_(cx, t, cx.vel.x);
        if (cx.jumpEdge && standing_clear)
        {
            cx.probes.refreshAll(cx, t.probeOffsets);
            if (DoJump(cx, t))
            {
                cx.basePose = static_cast<int>(STile::Airpause);
                return AvatarStatus::Hovering;
            }
        }

        if (h_hit.hit && standing_clear)
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::StandingA);
            return AvatarStatus::Standing;
        }

        if (_elapsed_frames < t.slidingFrames)
        {
            ++_elapsed_frames;
        }

        if (_elapsed_frames >= t.slidingFrames && standing_clear)
        {
            cx.animeStepper.reset();
            cx.basePose = static_cast<int>(STile::RunningA);
            return AvatarStatus::Running;
        }

        return AvatarStatus::Sliding;
    }

    void SlidingState::TickAnimationOnly(AnimeContext& ax, StateProvider*, const PlayerTuning&, double)
    {
        setPose_(ax);
    }

    Probes SlidingState::makeSlidingProbes_(const PlayerContext& cx, const PlayerTuning& t) const
    {
        PlayerProbes offsets = t.probeOffsets;
        const double direction = static_cast<double>(cx.facingLR);

        offsets.frontLineOffsetX = 11.0;
        offsets.rearLineOffsetX = -9.0;
        // SweepHorizontal samples three points; duplicate the upper point so the
        // sliding profile has exactly two effective samples inside the 16px gap.
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

    bool SlidingState::hasStandingClearance_(const PlayerContext& cx, const PlayerTuning& t, double dx) const
    {
        Probes standing_probes{ cx.probes.half };
        standing_probes.refreshAll(cx, t.probeOffsets);
        return !cx.terrain->SweepVertical(
            standing_probes,
            Vec2{ dx, -config::SystemConfig::kEpsilon }).hit;
    }

    void SlidingState::setPose_(PlayerContext& cx) const noexcept
    {
        cx.basePose = static_cast<int>(STile::Sliding);
    }

    void SlidingState::setPose_(AnimeContext& ax) const noexcept
    {
        ax.basePose = static_cast<int>(STile::Sliding);
    }
}
