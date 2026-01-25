#include "pch.h"

#include "AttackActionState.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::world::entity::avatar
{
    common::SpawnProjectileCommand AttackActionState::Update(
        PlayerContext& cx, StateProvider* in, const AttackTuning& tuning, bool can_spawn_projectile, double dt)
    {
        world::entity::common::SpawnProjectileCommand cmd{};

        // 1) 入力エッジは「攻撃中でも」毎回処理する（連射対応）
        if (in->JustPressed(JPBTN::B))
        {
            // 押すたびに攻撃ポーズをリセット（押し続けると攻撃モーションが継続/更新される）
            restartAttackPose_();

            // 弾はEntityManager側で上限判定して、撃てるときだけコマンドを返す
            if (can_spawn_projectile)
            {
                cmd.draw_layer = systems::view::Layer::Effects;
                cmd.sprite_id = cx.id;

                cmd.base_texture = tuning.projectile_base_texture;
                cmd.anim_frames = tuning.projectile_anim_frames;
                cmd.anim_fps = tuning.projectile_anim_fps;
                cmd.life_sec = tuning.projectile_life_sec;

                const bool is_left = (cx.facingLR == AvatarDirection::Left);
                const auto offset = is_left ? tuning.projectile_spawn_offset_px_left : tuning.projectile_spawn_offset_px_right;

                cmd.spawn_pos = cx.pos + offset;
                const double dir = static_cast<double>(cx.facingLR);
                cmd.velocity = foundation::math::Vec2{ tuning.projectile_speed_px_per_sec * dir, 0.0 };
            }
        }

        // 2) 攻撃ポーズ（+10）は一定時間だけ維持
        if (_is_attacking)
        {
            cx.textureAdd = tuning.attack_texture_add;

            _pose_time_sec += dt;
            if (_pose_time_sec >= tuning.attack_duration_sec)
            {
                _is_attacking = false;
                _pose_time_sec = 0.0;
            }
        }

        return cmd;
    }

    bool AttackActionState::IsAttacking() const noexcept
    {
        return _is_attacking;
    }

    Vec2 AttackActionState::GetRockBusterOffset(int texture, AvatarDirection facingLR) const noexcept
    {
        // 備考: ロックバスターのテクスチャは、胴体の部分とアームの部分が分割されてタイル配置されている。
        // 攻撃時(_is_attacking is true)はこれを結合してプレイヤーのグラフィックを生成する必要がある。
        // それを実現するため、プレイヤーのテクスチャに対するアーム部分のオフセットを計算し、その座標値を返す。
        Vec2 offset{ 0.0, 0.0 };
        double x_temp = 0.0;
        double y_temp = 0.0;
        switch (texture)
        {
        case 11:
        case 12:
        case 13:
            x_temp = 28.0;
            y_temp = 10.0;
            break;
        case 14:
        case 15:
            x_temp = 24.0;
            y_temp = 11.0;
            break;
        case 16:
            x_temp = 24.0;
            y_temp = 9.0;
            break;
        case 17:
            x_temp = 25.0;
            y_temp = 9.0;
            break;
        case 30:
        case 32:
            x_temp = 24.0;
            y_temp = 10.0;
            break;
        case 31:
        case 33:
            y_temp = 10.0;
            break;
        case 51:
        case 52:
        case 53:
            x_temp = -4.0;
            y_temp = 10.0;
            break;
        case 54:
        case 55:
            y_temp = 11.0;
            break;
        case 56:
            y_temp = 9.0;
            break;
        case 57:
            x_temp = -1.0;
            y_temp = 9.0;
            break;
        default:
            break;
        }

        offset.x = x_temp;
        offset.y = y_temp;
        return offset;
    }

    void AttackActionState::restartAttackPose_() noexcept
    {
        _is_attacking = true;
        _pose_time_sec = 0.0;
    }
}