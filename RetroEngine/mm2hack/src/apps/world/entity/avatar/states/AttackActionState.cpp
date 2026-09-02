#include "pch.h"

#include "AttackActionState.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/abilities/RockBusterOffsetTable.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerFrameOutput.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    using StateProvider = core::assembly::StateProvider;

    void AttackActionState::PreUpdate(PlayerContext& cx, StateProvider* in, bool can_spawn) noexcept
    {
        _can_spawn = can_spawn;
        if (in->JustPressed(JPBTN::B))
        {
            _charging = true;
            _charge_frames = 0;
            if (can_spawn)
            {
                restartAttackPose_(true);
            }
        }

        if (!_is_attacking) return;

        // If attacking, lock climbing movement.
        cx.lockClimbMove = true;

        // Update facing direction based on left/right input even while attacking.
        if (in->IsPressed(JPBTN::LEFT))  cx.facingLR = AvatarDirection::Left;
        if (in->IsPressed(JPBTN::RIGHT)) cx.facingLR = AvatarDirection::Right;
    }

    ActionUpdateResult AttackActionState::PostUpdate(PlayerContext& cx, StateProvider* in, const AttackTuning& tuning, double dt)
    {
        using namespace abilities;
        ActionUpdateResult result{};

        if (_charging && in->IsPressed(JPBTN::B))
        {
            ++_charge_frames;
        }

        if (_charging && in->JustReleased(JPBTN::B))
        {
            requestChargedShot_(tuning);
            _charging = false;
            _charge_frames = 0;
        }

        const auto charge_phase = chargePhase_(tuning);
        std::uint32_t phase_frames = 0;
        if (charge_phase == ChargePhase::Level1)
        {
            phase_frames = _charge_frames - tuning.level1ChargeFrames;
        }
        else if (charge_phase == ChargePhase::Level2)
        {
            phase_frames = _charge_frames - tuning.level2ChargeFrames;
        }
        cx.output.charge = ChargeStatus{ charge_phase, _charge_frames, phase_frames };

        if (_fire_requested)
        {
            result.spawnProjectile.emplace();
            result.spawnProjectile->drawLayer = systems::view::Layer::Effects;
            result.spawnProjectile->spriteId = _id;
            result.spawnProjectile->visual = _requested_visual;

            const bool is_left = (cx.facingLR == AvatarDirection::Left);
            result.spawnProjectile->baseTexture = tuning.rockBusterTexture + (is_left ? tuning.facingOffsetLeft : tuning.facingOffsetRight);
            result.spawnProjectile->animFrames = _requested_visual == common::ProjectileVisual::Normal
                ? tuning.projectileAnimFrames
                : 1;
            result.spawnProjectile->animFps = tuning.projectileAnimFps;
            result.spawnProjectile->lifeSec = tuning.projectileLifeSec;

            const auto offset = is_left ? tuning.projectileSpawnOffsetPxLeft : tuning.projectileSpawnOffsetPxRight;
            result.spawnProjectile->spawnPos = cx.pos + offset;
            const double dir = static_cast<double>(cx.facingLR);
            double speed = tuning.projectileSpeedPxPerSec;
            if (_requested_visual == common::ProjectileVisual::ChargeLevel1)
            {
                speed = tuning.chargeLevel1SpeedPxPerSec;
            }
            else if (_requested_visual == common::ProjectileVisual::ChargeLevel2)
            {
                speed = tuning.chargeLevel2SpeedPxPerSec;
            }
            result.spawnProjectile->velocity = foundation::math::Vec2{ speed * dir, 0.0 };

            cx.output.PushEvent(
                _requested_visual == common::ProjectileVisual::ChargeLevel2
                    ? PlayerEventType::FiredMaxChargeShot
                    : PlayerEventType::FiredRockBuster);

            _fire_requested = false;
        }

        if (_is_attacking)
        {
            result.textureAdd = tuning.attackTextureAdd;
            result.rockBuster.visible = true;
            result.rockBuster.armTexture = (cx.facingLR == AvatarDirection::Left) ? rb_tuning.arm_texture_left : rb_tuning.arm_texture_right;
            result.rockBuster.offset = find_rock_buster_offset_by_base_pose(cx.basePose, cx.facingLR);

            _pose_time_sec += dt;
            if (_pose_time_sec >= tuning.attackDurationSec)
            {
                finishAttackPose_();
            }
        }

        return result;
    }

    bool AttackActionState::IsAttacking() const noexcept
    {
        return _is_attacking;
    }

    void AttackActionState::Cancel() noexcept
    {
        _fire_requested = false;
        _charging = false;
        _charge_frames = 0;
        finishAttackPose_();
    }

    void AttackActionState::TickAnimationOnly(AnimeContext& ax, const AttackTuning& tuning, RockBusterDrawInfo& out_rb) const noexcept
    {
        using namespace abilities;
        out_rb.visible = false;

        if (!_is_attacking) return;

        ax.textureAdd += tuning.attackTextureAdd;
        out_rb.visible = true;
        out_rb.armTexture = (ax.facingLR == AvatarDirection::Left) ? rb_tuning.arm_texture_left : rb_tuning.arm_texture_right;
        out_rb.offset = find_rock_buster_offset_by_base_pose(ax.basePose, ax.facingLR);
    }

    void AttackActionState::restartAttackPose_(bool request_normal_shot) noexcept
    {
        _is_attacking = true;
        _pose_time_sec = 0.0;
        if (request_normal_shot)
        {
            _requested_visual = common::ProjectileVisual::Normal;
            _fire_requested = true;
        }
    }

    void AttackActionState::finishAttackPose_() noexcept
    {
        _is_attacking = false;
        _pose_time_sec = 0.0;
    }

    ChargePhase AttackActionState::chargePhase_(const AttackTuning& tuning) const noexcept
    {
        if (!_charging) return ChargePhase::Idle;
        if (_charge_frames >= tuning.level2ChargeFrames) return ChargePhase::Level2;
        if (_charge_frames >= tuning.level1ChargeFrames) return ChargePhase::Level1;
        return ChargePhase::Warmup;
    }

    void AttackActionState::requestChargedShot_(const AttackTuning& tuning) noexcept
    {
        if (!_can_spawn || _charge_frames < tuning.level1ChargeFrames) return;

        _requested_visual = _charge_frames >= tuning.level2ChargeFrames
            ? common::ProjectileVisual::ChargeLevel2
            : common::ProjectileVisual::ChargeLevel1;
        _fire_requested = true;
        restartAttackPose_();
    }
}
