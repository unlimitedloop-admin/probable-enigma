//==============================================================================
//
//  Project: mm2hack
//  EnemyEntity.h
//
//  Generic, data-driven enemy. One class covers every EnemyKind for this
//  first pass: sprite id, palette variant (color), toughness (resistance),
//  and a movement speed multiplier are all supplied per spawn rather than
//  hardcoded per kind. Movement is a simple left-right patrol for now;
//  per-kind AI (chase, hide, fly patterns, ...) is future work.
//
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <cstdint>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/combat/DamageTable.h"
#include "apps/systems/combat/HealthComponent.h"
#include "apps/systems/combat/IDamageable.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"
#include "apps/world/entity/IEntity.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::enemy
{
    struct EnemyEntityState final
    {
        EntityKinematicState kinematic{};
        EnemyKind kind{ EnemyKind::Met };
        std::int32_t base_texture{};
        std::int32_t palette_variant{};
        // 0 = invincible (immune to every weapon, HP never moves); N = dies
        // after N hits' worth of accumulated normal-shot power.
        std::int32_t toughness{ 1 };
        std::int32_t hp{ 1 };                          // Current HP (meaningless/always full when toughness == 0)
        foundation::math::Vec2 half_size{ 8.0, 8.0 };
        double spawn_x{};                              // Patrol center; independent of the live, moving pos.x
        double move_speed_px_per_sec{};                // Already includes the spawn-time speed scale
        std::int32_t facing{ 1 };                       // +1 = moving right, -1 = moving left

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    // Basic data-driven enemy: same shape for every EnemyKind at this stage --
    // per-kind differences are constructor parameters, not subclasses.
    class EnemyEntity final :
        public EntityBase,
        public systems::physics::ICollider,
        public systems::combat::IDamageable
    {
        // NOTE: no `Layer` alias here -- it would collide with ICollider::Layer() below.
        using RectF = foundation::math::RectF;
        using Vec2 = foundation::math::Vec2;
        using CollisionLayer = systems::physics::CollisionLayer;
        using TileAttribute = systems::physics::TileAttribute;
        using DamageTable = systems::combat::DamageTable;

    public:
        static constexpr std::uint16_t kStateVersion{ 1 };
        // Basic patrol tuning (px/sec at scale=1.0, and the +-range around
        // spawn_x it walks before turning). Not yet per-kind; a placeholder
        // until real per-kind movement AI lands.
        static constexpr double kDefaultPatrolSpeedPxPerSec{ 20.0 };
        static constexpr double kDefaultPatrolRangePx{ 24.0 };

        // Fresh placement (level/test spawn).
        // `palette_variant`: forwarded to SpriteManager::UseByIdVariant() each
        // frame -- the "color" knob, picking among the sheet's baked-in palettes.
        // `toughness`: 0 = invincible, N = dies after N hits' worth of
        // accumulated normal-shot power (1 = one-hit kill).
        // `move_speed_scale`: multiplies kDefaultPatrolSpeedPxPerSec.
        EnemyEntity(
            EnemyKind kind,
            Vec2 spawn_pos,
            rendering::sprite::SpriteManager::Id sprite_id,
            int base_texture,
            int palette_variant,
            int toughness,
            Vec2 half_size,
            double move_speed_scale = 1.0);
        // Reconstruction from saved state (kind/toughness/etc. all come from `state`).
        EnemyEntity(
            const EnemyEntityState& state,
            rendering::sprite::SpriteManager::Id sprite_id);

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Main update (IUpdatable) -- left-right patrol between spawn_x +- range
        void Update(const systems::view::ViewState* view, double dt) override;
        // Render (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::Enemy; }
        [[nodiscard]] std::uint16_t StateComponentVersion() const noexcept override { return kStateVersion; }
        bool SaveState(core::save::StateWriter& writer) const override;
        [[nodiscard]] EnemyEntityState CaptureState() const noexcept;
        bool RestoreState(const EnemyEntityState& state) noexcept;

        [[nodiscard]] EnemyKind Kind() const noexcept { return _kind; }

        // ICollider
        IEntity& OwnerEntity() noexcept override { return *this; }
        const IEntity& OwnerEntity() const noexcept override { return *this; }
        RectF Bounds() const override;
        bool IsCollidable() const noexcept override { return IsAlive(); }
        CollisionLayer Layer() const noexcept override { return CollisionLayer::Enemy; }
        void OnTileCollision(const Vec2& normal, TileAttribute attr) override { (void)normal; (void)attr; }
        void OnEntityCollision(IEntity& other) override;

        // IDamageable
        bool ApplyAttack(const systems::physics::IAttackInfo& attack) noexcept override;
        [[nodiscard]] int CurrentHP() const noexcept override { return _health.CurrentHP(); }
        [[nodiscard]] int MaxHP() const noexcept override { return _health.MaxHP(); }
        [[nodiscard]] bool IsDead() const noexcept override { return _health.IsDead(); }

    private:
        EnemyKind _kind{ EnemyKind::Met };
        rendering::sprite::SpriteManager::Id _id{};    // Object sprite id
        int _base_texture{ 0 };                         // Base texture index
        int _palette_variant{ 0 };                      // Per-instance color (SpriteManager variant index)
        Vec2 _half{ 8.0, 8.0 };                         // Half-size (used for both drawing and hit judgement)
        systems::combat::HealthComponent _health{};     // HP + per-weapon resistance
        int _toughness{ 1 };                            // Raw tuning value driving _health's (max_hp, table); saved as-is

        double _spawn_x{ 0.0 };                         // Patrol center
        double _move_speed{ 0.0 };                      // px/sec, already includes the spawn-time scale
        int _facing{ 1 };                                // +1 right, -1 left
    };
}
