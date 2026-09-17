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
            writer.WriteI32(base_texture) &&
            writer.WriteI32(facing_texture_offset_left) &&
            writer.WriteI32(palette_variant) &&
            writer.WriteI32(toughness) &&
            writer.WriteI32(hp) &&
            writer.WriteF64(half_size.x) &&
            writer.WriteF64(half_size.y) &&
            writer.WriteF64(spawn_x) &&
            writer.WriteF64(move_speed_px_per_sec) &&
            writer.WriteI32(facing);
    }

    bool EnemyEntityState::Load(core::save::StateReader& reader)
    {
        EnemyEntityState loaded{};
        std::uint16_t encoded_kind{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadU16(encoded_kind) ||
            !reader.ReadI32(loaded.base_texture) ||
            !reader.ReadI32(loaded.facing_texture_offset_left) ||
            !reader.ReadI32(loaded.palette_variant) ||
            !reader.ReadI32(loaded.toughness) ||
            !reader.ReadI32(loaded.hp) ||
            !reader.ReadF64(loaded.half_size.x) ||
            !reader.ReadF64(loaded.half_size.y) ||
            !reader.ReadF64(loaded.spawn_x) ||
            !reader.ReadF64(loaded.move_speed_px_per_sec) ||
            !reader.ReadI32(loaded.facing))
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
        constexpr int kMaximumPaletteVariant = 16;

        return kinematic.IsValid() &&
            kind <= EnemyKind::FlyBoy &&
            base_texture >= 0 && base_texture <= 65'535 &&
            facing_texture_offset_left >= -65'535 && facing_texture_offset_left <= 65'535 &&
            palette_variant >= 0 && palette_variant <= kMaximumPaletteVariant &&
            toughness >= 0 && toughness <= 1'000 &&
            hp >= 1 && hp <= std::max(1, toughness) &&
            std::isfinite(half_size.x) && half_size.x > 0.0 && half_size.x <= kMaximumHalfSize &&
            std::isfinite(half_size.y) && half_size.y > 0.0 && half_size.y <= kMaximumHalfSize &&
            std::isfinite(spawn_x) && std::abs(spawn_x) <= kMaximumCoordinate &&
            std::isfinite(move_speed_px_per_sec) &&
            move_speed_px_per_sec >= 0.0 && move_speed_px_per_sec <= kMaximumSpeed &&
            (facing == 1 || facing == -1);
    }

    EnemyEntity::EnemyEntity(
        EnemyKind kind,
        Vec2 spawn_pos,
        rendering::sprite::SpriteManager::Id sprite_id,
        int base_texture,
        int facing_texture_offset_left,
        int palette_variant,
        int toughness,
        Vec2 half_size,
        double move_speed_scale)
        : _kind(kind), _id(sprite_id), _base_texture(base_texture),
          _facing_texture_offset_left(facing_texture_offset_left),
          _palette_variant(palette_variant), _half(half_size), _toughness(toughness)
    {
        pos = spawn_pos;
        _spawn_x = spawn_pos.x;
        _move_speed = kDefaultPatrolSpeedPxPerSec * std::max(0.0, move_speed_scale);

        const auto [max_hp, table] = HealthForToughness(toughness);
        _health = systems::combat::HealthComponent(max_hp, table);
    }

    EnemyEntity::EnemyEntity(
        const EnemyEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        RestoreState(state);
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
            .base_texture = _base_texture,
            .facing_texture_offset_left = _facing_texture_offset_left,
            .palette_variant = _palette_variant,
            .toughness = _toughness,
            .hp = _health.CurrentHP(),
            .half_size = _half,
            .spawn_x = _spawn_x,
            .move_speed_px_per_sec = _move_speed,
            .facing = _facing,
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
        _base_texture = state.base_texture;
        _facing_texture_offset_left = state.facing_texture_offset_left;
        _palette_variant = state.palette_variant;
        _half = state.half_size;
        _spawn_x = state.spawn_x;
        _move_speed = state.move_speed_px_per_sec;
        _facing = state.facing;
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
        pos.x += static_cast<double>(_facing) * _move_speed * dt;

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

    void EnemyEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        const auto& view = *ctx.view;
        const int x = static_cast<int>(pos.x - view.viewWorldX - _half.x);
        const int y = static_cast<int>(pos.y - view.viewWorldY - _half.y);

        const int texture = _base_texture + (_facing < 0 ? _facing_texture_offset_left : 0);

        auto& res = runtime::GameContext::GetInstance().GetResourceManager();
        res.GetSpriteManager().UseByIdVariant(_id, _palette_variant, texture, x, y);
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
