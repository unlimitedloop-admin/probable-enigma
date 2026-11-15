#include "pch.h"

#include "SetbackState.h"

#include <algorithm>

#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
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

    void SetbackState::OnEnter(PlayerContext& cx, StateProvider*, const PlayerTuning& t)
    {
        _elapsed_frames = 0;
        _airborne = !cx.onGround;
        // "Still rising" is judged off the real vel.y at the moment of
        // impact, but the launch itself always uses the full jumpImpulse --
        // a partial hit late in the ascent (vel.y just past the threshold)
        // relaunching at full strength reads as a proper knockback punch,
        // rather than a barely-there continuation of whatever was left of
        // the original jump. Gravity decays it from there every frame
        // (abilities::apply_gravity, Update()), same as a real jump arc, and
        // it's free to turn into a fall before the 28 frames are up.
        _rising = _airborne && cx.vel.y < kAirRiseThreshold;

        cx.vel = {};
        if (_rising) { cx.vel.y = t.jumpImpulse; }
        cx.animeStepper.reset();

        cx.basePose = static_cast<int>(_airborne ? kAirCycle[0] : kGroundCycle[0]);
        cx.damageEffectTile = kEffectBaseTile + (cx.facingLR == AvatarDirection::Left ? kEffectLeftOffset : 0);
    }

    AvatarStatus SetbackState::Update(PlayerContext& cx, StateProvider*, const PlayerTuning& t, double)
    {
        if (_elapsed_frames < kDurationFrames)
        {
            ++_elapsed_frames;
        }

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
            if (_rising)
            {
                // Same decay normal jump physics uses -- not a fixed rate.
                // Naturally turns into falling once gravity eats through it.
                abilities::apply_gravity(cx.vel, t.gravity, t.terminalVelocity);
            }
            else
            {
                cx.vel.y = kAirFallPxPerFrame;
            }

            const auto v_hit = cx.terrain->SweepVertical(cx.probes, cx.vel);
            if (v_hit.hit)
            {
                cx.vel.y = v_hit.maxDistanceY;
                const bool now_grounded = (v_hit.kind == systems::physics::VHitKind::Floor);
                if (now_grounded && !cx.onGround)
                {
                    // Landed before the 28 frames were up -- convert to ground
                    // mode (both movement and the tile cycle below key off
                    // _airborne) and keep going for the remaining frames,
                    // retreating like a knockback that started grounded,
                    // instead of ending early or handing off to Hovering
                    // while already touching the floor.
                    _airborne = false;
                }
                cx.onGround = now_grounded;
            }
        }

        const auto& body_cycle = _airborne ? kAirCycle : kGroundCycle;
        const int body_index = (_elapsed_frames / kBodyFramesPerTile) % 2;
        cx.basePose = static_cast<int>(body_cycle[body_index]);

        const int effect_index = std::min(3, static_cast<int>(_elapsed_frames / kEffectFramesPerTile));
        cx.damageEffectTile = kEffectBaseTile + effect_index + (cx.facingLR == AvatarDirection::Left ? kEffectLeftOffset : 0);

        if (_elapsed_frames >= kDurationFrames)
        {
            return _airborne ? AvatarStatus::Hovering : AvatarStatus::Standing;
        }
        return AvatarStatus::Setback;
    }
}
