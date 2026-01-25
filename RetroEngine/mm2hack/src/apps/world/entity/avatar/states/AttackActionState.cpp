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

        // 1) Input edges are processed every time, even during attacks. (for rapid fire)
        if (in->JustPressed(JPBTN::B))
        {
            auto updateFacing = [&](void) noexcept
            {
                if (in->IsPressed(JPBTN::LEFT))
                {
                    cx.facingLR = AvatarDirection::Left;
                }
                else if (in->IsPressed(JPBTN::RIGHT))
                {
                    cx.facingLR = AvatarDirection::Right;
                }
            };

            // Attack pose restart on input edge.
            restartAttackPose_();

            updateFacing();     // Update 'cx.facingLR' based on current input.

            // Projectile spawn command.
            if (can_spawn_projectile)
            {
                cmd.drawLayer = systems::view::Layer::Effects;
                cmd.spriteId = cx.id;

                cmd.baseTexture = tuning.projectile_base_texture;
                cmd.animFrames = tuning.projectile_anim_frames;
                cmd.animFps = tuning.projectile_anim_fps;
                cmd.lifeSec = tuning.projectile_life_sec;

                const bool is_left = (cx.facingLR == AvatarDirection::Left);
                const auto offset = is_left ? tuning.projectile_spawn_offset_px_left : tuning.projectile_spawn_offset_px_right;

                cmd.spawnPos = cx.pos + offset;
                const double dir = static_cast<double>(cx.facingLR);
                cmd.velocity = foundation::math::Vec2{ tuning.projectile_speed_px_per_sec * dir, 0.0 };
            }
        }

        // 2) The attack pose (+10) is maintained for a certain period of time.
        if (_is_attacking)
        {
            cx.textureAdd = tuning.attack_texture_add;
            cx.lockClimbMove = true;    // Don't allow climbing movement during attack.

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

    void AttackActionState::TickAnimationOnly(AnimeContext& ax, const AttackTuning& tuning, double dt) const noexcept
    {
        // No state update needed
        if (_is_attacking)
        {
            // Still in attack pose
            ax.textureAdd = tuning.attack_texture_add;
        }
    }

    Vec2 AttackActionState::GetRockBusterOffset(int texture, AvatarDirection facingLR) const noexcept
    {
        // NOTE: The Rock Buster's texture is divided into the torso part and the arm part, which are arranged as tiles.
        // During an attack (_is_attacking is true), these need to be combined to generate the player's graphic.
        // To achieve this, calculate the offset of the arm part relative to the player's texture and return that coordinate value.
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
        case 31:
        case 32:
        case 33:
            x_temp = 24.0;
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
        case 70:
        case 71:
        case 72:
        case 73:
            y_temp = 10.0;
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