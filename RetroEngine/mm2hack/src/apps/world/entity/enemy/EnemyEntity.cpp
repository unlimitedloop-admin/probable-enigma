#include "pch.h"

#include "EnemyEntity.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/IAttackInfo.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::enemy
{
    using systems::combat::DamageTable;
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    namespace
    {
        // 0 = invincible (every weapon immune, HP never moves); N = dies after N
        // hits' worth of accumulated normal-shot power. Shared by both
        // constructors so a fresh spawn and a save/load restore always agree.
        std::pair<int, DamageTable> HealthForToughness(int toughness) noexcept
        {
            if (toughness <= 0)
            {
                return { 1, DamageTable::Invincible() };
            }
            return { toughness, DamageTable::Neutral() };
        }
    }

    bool EnemyEntityState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() && kinematic.Save(writer) &&
            writer.WriteU16(static_cast<std::uint16_t>(kind)) &&
            writer.WriteI32(palette_preset_index) &&
            writer.WriteI32(facing_texture_offset_left) &&
            writer.WriteI32(toughness) &&
            writer.WriteI32(hp) &&
            writer.WriteF64(half_size.x) &&
            writer.WriteF64(half_size.y) &&
            writer.WriteF64(spawn_x) &&
            writer.WriteF64(move_speed_px_per_sec) &&
            writer.WriteI32(facing) &&
            writer.WriteI32(anim_state_index) &&
            writer.WriteI32(anim_frame_index) &&
            writer.WriteI32(anim_frame_elapsed) &&
            writer.WriteI32(anim_state_elapsed) &&
            writer.WriteF64(gravity_per_frame) &&
            writer.WriteF64(terminal_velocity_per_frame) &&
            writer.WriteF64(gravity_vel_y) &&
            writer.WriteBool(on_ground);
    }

    bool EnemyEntityState::Load(core::save::StateReader& reader)
    {
        EnemyEntityState loaded{};
        std::uint16_t encoded_kind{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadU16(encoded_kind) ||
            !reader.ReadI32(loaded.palette_preset_index) ||
            !reader.ReadI32(loaded.facing_texture_offset_left) ||
            !reader.ReadI32(loaded.toughness) ||
            !reader.ReadI32(loaded.hp) ||
            !reader.ReadF64(loaded.half_size.x) ||
            !reader.ReadF64(loaded.half_size.y) ||
            !reader.ReadF64(loaded.spawn_x) ||
            !reader.ReadF64(loaded.move_speed_px_per_sec) ||
            !reader.ReadI32(loaded.facing) ||
            !reader.ReadI32(loaded.anim_state_index) ||
            !reader.ReadI32(loaded.anim_frame_index) ||
            !reader.ReadI32(loaded.anim_frame_elapsed) ||
            !reader.ReadI32(loaded.anim_state_elapsed) ||
            !reader.ReadF64(loaded.gravity_per_frame) ||
            !reader.ReadF64(loaded.terminal_velocity_per_frame) ||
            !reader.ReadF64(loaded.gravity_vel_y) ||
            !reader.ReadBool(loaded.on_ground))
        {
            return false;
        }
        loaded.kind = static_cast<EnemyKind>(encoded_kind);
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool EnemyEntityState::IsValid() const noexcept
    {
        constexpr double kMaximumHalfSize = 256.0;
        constexpr double kMaximumCoordinate = 1'000'000.0;
        constexpr double kMaximumSpeed = 10'000.0;
        constexpr int kMaximumAnimCounter = 36'000;

        return kinematic.IsValid() &&
            kind <= EnemyKind::FlyBoy &&
            palette_preset_index >= 0 && palette_preset_index <= 255 &&
            facing_texture_offset_left >= -65'535 && facing_texture_offset_left <= 65'535 &&
            toughness >= 0 && toughness <= 1'000 &&
            hp >= 1 && hp <= std::max(1, toughness) &&
            std::isfinite(half_size.x) && half_size.x > 0.0 && half_size.x <= kMaximumHalfSize &&
            std::isfinite(half_size.y) && half_size.y > 0.0 && half_size.y <= kMaximumHalfSize &&
            std::isfinite(spawn_x) && std::abs(spawn_x) <= kMaximumCoordinate &&
            std::isfinite(move_speed_px_per_sec) &&
            move_speed_px_per_sec >= 0.0 && move_speed_px_per_sec <= kMaximumSpeed &&
            (facing == 1 || facing == -1) &&
            anim_state_index >= 0 && anim_state_index <= kMaximumAnimCounter &&
            anim_frame_index >= 0 && anim_frame_index <= kMaximumAnimCounter &&
            anim_frame_elapsed >= 0 && anim_frame_elapsed <= kMaximumAnimCounter &&
            anim_state_elapsed >= 0 && anim_state_elapsed <= kMaximumAnimCounter &&
            std::isfinite(gravity_per_frame) && gravity_per_frame >= 0.0 && gravity_per_frame <= 100.0 &&
            std::isfinite(terminal_velocity_per_frame) &&
            terminal_velocity_per_frame >= 0.0 && terminal_velocity_per_frame <= 1'000.0 &&
            std::isfinite(gravity_vel_y) && std::abs(gravity_vel_y) <= 1'000.0;
    }

    EnemyEntity::EnemyEntity(
        EnemyKind kind,
        Vec2 spawn_pos,
        rendering::sprite::SpriteManager::Id sprite_id,
        int palette_preset_index,
        const animation::EnemyAnimationDef* animation_def,
        int facing_texture_offset_left,
        int toughness,
        Vec2 half_size,
        double move_speed_scale,
        double gravity_scale)
        : _kind(kind), _id(sprite_id), _palette_preset_index(palette_preset_index),
          _facing_texture_offset_left(facing_texture_offset_left), _half(half_size),
          _toughness(toughness)
    {
        pos = spawn_pos;
        _spawn_x = spawn_pos.x;
        _move_speed = kDefaultPatrolSpeedPxPerSec * std::max(0.0, move_speed_scale);

        const double scale = std::max(0.0, gravity_scale);
        _gravity = systems::physics::SimpleGravityBody(
            kDefaultGravityPerFrame * scale, kDefaultTerminalVelocityPerFrame * scale);

        const auto [max_hp, table] = HealthForToughness(toughness);
        _health = systems::combat::HealthComponent(max_hp, table);

        if (animation_def != nullptr)
        {
            _animator = animation::AnimationStatePlayer(*animation_def);
        }
    }

    EnemyEntity::EnemyEntity(
        const EnemyEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id,
        const animation::EnemyAnimationDef* animation_def)
        : _id(sprite_id)
    {
        RestoreState(state);
        if (animation_def != nullptr)
        {
            _animator.RestoreState(
                state.anim_state_index, state.anim_frame_index,
                state.anim_frame_elapsed, state.anim_state_elapsed,
                *animation_def);
        }
    }

    bool EnemyEntity::SaveState(core::save::StateWriter& writer) const
    {
        return CaptureState().Save(writer);
    }

    EnemyEntityState EnemyEntity::CaptureState() const noexcept
    {
        return EnemyEntityState{
            .kinematic = CaptureKinematicState(),
            .kind = _kind,
            .palette_preset_index = _palette_preset_index,
            .facing_texture_offset_left = _facing_texture_offset_left,
            .toughness = _toughness,
            .hp = _health.CurrentHP(),
            .half_size = _half,
            .spawn_x = _spawn_x,
            .move_speed_px_per_sec = _move_speed,
            .facing = _facing,
            .anim_state_index = _animator.StateIndex(),
            .anim_frame_index = _animator.FrameIndex(),
            .anim_frame_elapsed = _animator.FrameElapsed(),
            .anim_state_elapsed = _animator.StateElapsed(),
            .gravity_per_frame = _gravity.GravityPerFrame(),
            .terminal_velocity_per_frame = _gravity.TerminalVelocityPerFrame(),
            .gravity_vel_y = _gravity.VerticalVelocity(),
            .on_ground = _gravity.IsOnGround(),
        };
    }

    bool EnemyEntity::RestoreState(const EnemyEntityState& state) noexcept
    {
        if (!state.IsValid() || !RestoreKinematicState(state.kinematic))
        {
            return false;
        }

        // Toughness (not the saved HealthComponent bytes -- there are none here)
        // is the single source of truth for (max_hp, resistance table), so a
        // restored invincible enemy is guaranteed to still be invincible.
        const auto [max_hp, table] = HealthForToughness(state.toughness);
        const systems::combat::HealthComponentState hp_state{
            max_hp, std::clamp(state.hp, 1, max_hp)
        };
        if (!hp_state.IsValid() || !_health.RestoreState(hp_state, table))
        {
            return false;
        }

        _kind = state.kind;
        _toughness = state.toughness;
        _palette_preset_index = state.palette_preset_index;
        _facing_texture_offset_left = state.facing_texture_offset_left;
        _half = state.half_size;
        _spawn_x = state.spawn_x;
        _move_speed = state.move_speed_px_per_sec;
        _facing = state.facing;

        _gravity = systems::physics::SimpleGravityBody(
            state.gravity_per_frame, state.terminal_velocity_per_frame);
        if (!_gravity.RestoreState({ state.gravity_vel_y, state.on_ground }))
        {
            return false;
        }
        // NOTE: _animator itself is (re)attached by the reconstruction
        // constructor, which has the EnemyAnimationDef this needs and this
        // method doesn't. A fresh RestoreState() call on an already-live entity
        // (not currently done anywhere) would leave the animator as-is.
        return true;
    }

    Layer EnemyEntity::DrawLayer() const noexcept
    {
        return Layer::Actors;
    }

    void EnemyEntity::Update(const ViewState* view, double dt)
    {
        (void)view;

        if (!IsAlive())
        {
            return;
        }

        // Basic left-right patrol, bouncing between spawn_x +- range. Real
        // per-kind movement (ledge/wall detection, chase, fly patterns, ...) is
        // future work; this only exists to exercise the move-speed knob.
        // Gated by the current animation state (see AnimationState::
        // allow_movement) -- e.g. Met holds still while hidden under its helmet.
        if (_animator.AllowsMovement())
        {
            pos.x += static_cast<double>(_facing) * _move_speed * _animator.MoveSpeedMultiplier() * dt;

            const double left = _spawn_x - kDefaultPatrolRangePx;
            const double right = _spawn_x + kDefaultPatrolRangePx;
            if (pos.x <= left)
            {
                pos.x = left;
                _facing = 1;
            }
            else if (pos.x >= right)
            {
                pos.x = right;
                _facing = -1;
            }
        }

        // Vertical physics: gravity + ground snap. The player's own sprite
        // sits 1px into the floor by design (confirmed correct in this
        // project), so a ground-resting enemy gets the same nudge whenever
        // SimpleGravityBody reports a (physics-flush) landing.
        constexpr double kVisualFloorSinkPx = 1.0;
        const double resolved_bottom_y = _gravity.Tick(_terrain, pos.x, pos.y + _half.y);
        pos.y = resolved_bottom_y - _half.y + (_gravity.IsOnGround() ? kVisualFloorSinkPx : 0.0);

        _animator.Tick(animation::AnimationConditionInputs{ .grounded = _gravity.IsOnGround() });

        // A transition that just fired may carry a jump impulse (see
        // AnimationTransition::jump_impulse) -- e.g. Met's periodic hop while
        // walking. Applied after this tick's gravity resolution above so it
        // takes effect starting next frame, not retroactively this one.
        const double jump_impulse = _animator.LastJumpImpulse();
        if (jump_impulse != 0.0)
        {
            _gravity.Jump(jump_impulse);
        }
    }

    void EnemyEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        const auto& view = *ctx.view;
        const int x = static_cast<int>(pos.x - view.viewWorldX - _half.x);
        const int y = static_cast<int>(pos.y - view.viewWorldY - _half.y);

        const int texture = _animator.CurrentTile() +
            (_facing < 0 ? _facing_texture_offset_left : 0);

        auto& res = runtime::GameContext::GetInstance().GetResourceManager();
        res.GetSpriteManager().UseById(_id, texture, x, y);
    }

    EnemyEntity::RectF EnemyEntity::Bounds() const
    {
        return { pos.x - _half.x, pos.y - _half.y, _half.x * 2.0, _half.y * 2.0 };
    }

    void EnemyEntity::OnEntityCollision(IEntity& other)
    {
        if (!IsAlive())
        {
            return;
        }

        // Only a collider that carries an attack payload damages the enemy (a
        // player projectile today). Contact damage against the player -- this
        // enemy implementing IAttackInfo -- is a later step (player isn't
        // IDamageable yet either).
        auto* attack = dynamic_cast<systems::physics::IAttackInfo*>(&other);
        if (attack == nullptr)
        {
            return;
        }

        if (ApplyAttack(*attack))
        {
            // Destruction VFX/SFX and the non-lethal "hit" SE are both fired
            // generically from AbstractActionPhase off IDamageable -- nothing
            // more to do here than die.
            Kill();
        }
    }

    bool EnemyEntity::ApplyAttack(const systems::physics::IAttackInfo& attack) noexcept
    {
        return _health.ApplyAttack(attack);
    }
}
