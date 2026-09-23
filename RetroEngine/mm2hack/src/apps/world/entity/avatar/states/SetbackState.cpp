#include "pch.h"

#include "SetbackState.h"

#include <algorithm>

#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    namespace
    {
        // Ground: 81 -> 83 -> (repeat). Air: 84 -> 86 -> (repeat). The middle
        // pose (82/85) was dropped per follow-up feedback -- just a 2-tile flicker.
        constexpr STile kGroundCycle[2]{ STile::DamagedGroundA, STile::DamagedGroundC };
        constexpr STile kAirCycle[2]{ STile::DamagedAirA, STile::DamagedAirC };
        constexpr int kEffectBaseTile{ 8 }; // 8,9,10,11 -- 11 is blank by design (see header comment)
    }

    AvatarStatus SetbackState::Id() const noexcept
    {
        return AvatarStatus::Setback;
    }

    bool SetbackState::RestoreState(std::uint8_t elapsed_frames, bool airborne, bool rising) noexcept
    {
        _elapsed_frames = elapsed_frames;
        _airborne = airborne;
        _rising = rising;
        return true;
    }

    void SetbackState::OnEnter(PlayerContext& cx, StateProvider*, const PlayerTuning&)
    {
        _elapsed_frames = 0;
        _airborne = !cx.onGround;
        _rising = _airborne && cx.vel.y < kAirRiseThreshold;
        cx.vel = {};
        cx.animeStepper.reset();

        cx.basePose = static_cast<int>(_airborne ? kAirCycle[0] : kGroundCycle[0]);
        cx.damageEffectTile = kEffectBaseTile + (cx.facingLR == AvatarDirection::Left ? kEffectLeftOffset : 0);
    }

    AvatarStatus SetbackState::Update(PlayerContext& cx, StateProvider*, const PlayerTuning&, double)
    {
        if (_elapsed_frames < kDurationFrames)
        {
            ++_elapsed_frames;
        }

        // Only meaningful for the air branch: true the instant the falling
        // slam touches ground before the 28-frame timer runs out. Without
        // this, a knockback that started airborne would keep running the air
        // cycle (and later hand off to Hovering) even after it's visibly
        // sitting on the floor -- Hovering never sees a landing edge in that
        // case (onGround was already true on entry) and gets stuck.
        bool landed_early = false;

        if (!_airborne)
        {
            const double dir = -static_cast<double>(cx.facingLR); // retreat: opposite of facing
            const double requested_dx = kGroundRetreatPxPerFrame * dir;
            const auto h_hit = cx.terrain->SweepHorizontal(cx.probes, requested_dx);
            cx.vel.x = h_hit.hit ? h_hit.maxDistanceX : requested_dx;
            cx.vel.y = 0.0;
        }
        else
        {
            cx.vel.x = 0.0;
            cx.vel.y = _rising ? -kAirRisePxPerFrame : kAirFallPxPerFrame;
            const auto v_hit = cx.terrain->SweepVertical(cx.probes, cx.vel);
            if (v_hit.hit)
            {
                cx.vel.y = v_hit.maxDistanceY;
                const bool now_grounded = (v_hit.kind == systems::physics::VHitKind::Floor);
                landed_early = now_grounded && !cx.onGround;
                cx.onGround = now_grounded;
            }
        }

        const auto& body_cycle = _airborne ? kAirCycle : kGroundCycle;
        const int body_index = (_elapsed_frames / kBodyFramesPerTile) % 2;
        cx.basePose = static_cast<int>(body_cycle[body_index]);

        const int effect_index = std::min(3, static_cast<int>(_elapsed_frames / kEffectFramesPerTile));
        cx.damageEffectTile = kEffectBaseTile + effect_index + (cx.facingLR == AvatarDirection::Left ? kEffectLeftOffset : 0);

        if (landed_early)
        {
            return AvatarStatus::Standing;
        }
        if (_elapsed_frames >= kDurationFrames)
        {
            return _airborne ? AvatarStatus::Hovering : AvatarStatus::Standing;
        }
        return AvatarStatus::Setback;
    }
}
