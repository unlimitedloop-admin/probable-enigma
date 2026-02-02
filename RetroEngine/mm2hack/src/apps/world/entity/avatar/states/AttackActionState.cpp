#include "pch.h"

#include "AttackActionState.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/abilities/RockBusterOffsetTable.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    void AttackActionState::PreUpdate(PlayerContext& cx, StateProvider* in, bool can_spawn) noexcept
    {
        if (can_spawn && in->JustPressed(JPBTN::B))
        {
            restartAttackPose_();
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

        if (_fire_requested)
        {
            result.spawnProjectile.emplace();
            result.spawnProjectile->drawLayer = systems::view::Layer::Effects;
            result.spawnProjectile->spriteId = cx.weaponId;

            const bool is_left = (cx.facingLR == AvatarDirection::Left);
            result.spawnProjectile->baseTexture = tuning.rockBusterTexture + (is_left ? tuning.facingOffsetLeft : tuning.facingOffsetRight);
            result.spawnProjectile->animFrames = tuning.projectileAnimFrames;
            result.spawnProjectile->animFps = tuning.projectileAnimFps;
            result.spawnProjectile->lifeSec = tuning.projectileLifeSec;

            const auto offset = is_left ? tuning.projectileSpawnOffsetPxLeft : tuning.projectileSpawnOffsetPxRight;
            result.spawnProjectile->spawnPos = cx.pos + offset;
            const double dir = static_cast<double>(cx.facingLR);
            result.spawnProjectile->velocity = foundation::math::Vec2{ tuning.projectileSpeedPxPerSec * dir, 0.0 };

            _fire_requested = false;
        }

        if (_is_attacking)
        {
            result.textureAdd = tuning.attackTextureAdd;
            result.rockBuster.visible = true;
            result.rockBuster.armTexture = (cx.facingLR == AvatarDirection::Left) ? rb_tuning.arm_texture_left : rb_tuning.arm_texture_right;
            result.rockBuster.offset = FindRockBusterOffsetByBasePose(cx.basePose, cx.facingLR);

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

    void AttackActionState::TickAnimationOnly(AnimeContext& ax, const AttackTuning& tuning, double dt, RockBusterDrawInfo& out_rb) const noexcept
    {
        using namespace abilities;
        out_rb.visible = false;

        if (!_is_attacking) return;

        ax.textureAdd += tuning.attackTextureAdd;
        out_rb.visible = true;
        out_rb.armTexture = (ax.facingLR == AvatarDirection::Left) ? rb_tuning.arm_texture_left : rb_tuning.arm_texture_right;
        out_rb.offset = FindRockBusterOffsetByBasePose(ax.basePose, ax.facingLR);
    }

    void AttackActionState::restartAttackPose_() noexcept
    {
        _is_attacking = true;
        _pose_time_sec = 0.0;
        _fire_requested = true;
    }

    void AttackActionState::finishAttackPose_() noexcept
    {
        _is_attacking = false;
        _pose_time_sec = 0.0;
    }
}