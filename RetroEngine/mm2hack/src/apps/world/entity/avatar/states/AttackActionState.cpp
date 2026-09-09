#include "pch.h"

#include "AttackActionState.h"

#include <cmath>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/abilities/RockBusterOffsetTable.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerFrameOutput.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "core/assembly/StateProvider.h"
#include "core/save/StateIO.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    using StateProvider = core::assembly::StateProvider;

    bool AttackActionSnapshot::Save(core::save::StateWriter& writer) const
    {
        return IsValid() && writer.WriteBool(attacking) &&
            writer.WriteF64(pose_time_seconds) && writer.WriteBool(fire_requested) &&
            writer.WriteBool(charging) && writer.WriteU32(charge_frames) &&
            writer.WriteU8(static_cast<std::uint8_t>(requested_visual));
    }

    bool AttackActionSnapshot::Load(core::save::StateReader& reader)
    {
        AttackActionSnapshot loaded{};
        std::uint8_t encoded_visual{};
        if (!reader.ReadBool(loaded.attacking) ||
            !reader.ReadF64(loaded.pose_time_seconds) ||
            !reader.ReadBool(loaded.fire_requested) ||
            !reader.ReadBool(loaded.charging) ||
            !reader.ReadU32(loaded.charge_frames) ||
            !reader.ReadU8(encoded_visual))
        {
            return false;
        }
        loaded.requested_visual = static_cast<common::ProjectileVisual>(encoded_visual);
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool AttackActionSnapshot::IsValid() const noexcept
    {
        constexpr double kMaximumPoseSeconds = 60.0;
        constexpr std::uint32_t kMaximumChargeFrames = 60U * 60U * 60U;
        return std::isfinite(pose_time_seconds) && pose_time_seconds >= 0.0 &&
            pose_time_seconds <= kMaximumPoseSeconds &&
            (!fire_requested || attacking) &&
            (charging || charge_frames == 0) &&
            charge_frames <= kMaximumChargeFrames &&
            requested_visual >= common::ProjectileVisual::Normal &&
            requested_visual <= common::ProjectileVisual::ChargeLevel2;
    }

    void AttackActionState::PreUpdate(PlayerContext& cx, StateProvider* in, bool can_spawn) noexcept
    {
        _can_spawn = can_spawn;
        if (_reconcile_restored_charge_input)
        {
            _reconcile_restored_charge_input = false;
            if (!in->IsPressed(JPBTN::B))
            {
                _charging = false;
                _charge_frames = 0;
            }
        }

        if (!_charging && in->JustPressed(JPBTN::B))
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

    AttackActionSnapshot AttackActionState::CaptureState() const noexcept
    {
        return AttackActionSnapshot{
            .attacking = _is_attacking,
            .pose_time_seconds = _pose_time_sec,
            .fire_requested = _fire_requested,
            .charging = _charging,
            .charge_frames = _charge_frames,
            .requested_visual = _requested_visual,
        };
    }

    bool AttackActionState::RestoreState(const AttackActionSnapshot& state) noexcept
    {
        if (!state.IsValid())
        {
            return false;
        }
        _is_attacking = state.attacking;
        _pose_time_sec = state.pose_time_seconds;
        _fire_requested = state.fire_requested;
        _charging = state.charging;
        _reconcile_restored_charge_input = state.charging;
        _can_spawn = false;
        _charge_frames = state.charge_frames;
        _requested_visual = state.requested_visual;
        return true;
    }

    void AttackActionState::Cancel() noexcept
    {
        _fire_requested = false;
        _charging = false;
        _reconcile_restored_charge_input = false;
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
