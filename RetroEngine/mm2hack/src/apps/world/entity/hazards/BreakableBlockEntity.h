//==============================================================================
//
//  Project: mm2hack
//  BreakableBlockEntity.h
//
//  A static, destructible world object used to verify entity-vs-entity hit
//  detection (CollisionLayer::Trap) ahead of real enemies being implemented.
//
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <cstdint>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/systems/combat/DamageTable.h"
#include "apps/systems/combat/HealthComponent.h"
#include "apps/systems/combat/IDamageable.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/IEntity.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::hazards
{
    struct BreakableBlockEntityState final
    {
        EntityKinematicState kinematic{};
        std::int32_t tile_index{};
        systems::combat::HealthComponentState health{};
        foundation::math::Vec2 half_size{ 8.0, 8.0 };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    // Entity representing a destructible block (verification object for hit detection).
    class BreakableBlockEntity final :
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

        // Fresh placement (level/test spawn). `tileset_id` is the BG tileset to draw
        // from (typically the stage's own tileset, so the block matches the
        // surrounding art), `tile_index` the tile within it. `resistances` defaults
        // to full damage from every weapon; pass a tuned table for objects that
        // should resist (or be immune to) specific weapons.
        BreakableBlockEntity(
            Vec2 spawn_pos,
            rendering::bg::BGTileManager::Id tileset_id,
            int tile_index,
            int max_hp,
            Vec2 half_size,
            DamageTable resistances = DamageTable::Neutral());
        // Reconstruction from saved state. `tileset_id`/`resistances` come from the
        // level's spawn parameters, same as tile_index/half_size would for a real
        // level format -- they are not part of the saved bytes.
        BreakableBlockEntity(
            const BreakableBlockEntityState& state,
            rendering::bg::BGTileManager::Id tileset_id,
            DamageTable resistances = DamageTable::Neutral());

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Main update (IUpdatable) -- static object, only checks IsAlive()
        void Update(const systems::view::ViewState* view, double dt) override;
        // Render (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::BreakableBlock; }
        [[nodiscard]] std::uint16_t StateComponentVersion() const noexcept override { return kStateVersion; }
        bool SaveState(core::save::StateWriter& writer) const override;
        [[nodiscard]] BreakableBlockEntityState CaptureState() const noexcept;
        bool RestoreState(const BreakableBlockEntityState& state) noexcept;

        // ICollider
        IEntity& OwnerEntity() noexcept override { return *this; }
        const IEntity& OwnerEntity() const noexcept override { return *this; }
        RectF Bounds() const override;
        bool IsCollidable() const noexcept override { return IsAlive(); }
        CollisionLayer Layer() const noexcept override { return CollisionLayer::Trap; }
        void OnTileCollision(const Vec2& normal, TileAttribute attr) override { (void)normal; (void)attr; }
        void OnEntityCollision(IEntity& other) override;

        // IDamageable
        bool ApplyAttack(const systems::physics::IAttackInfo& attack) noexcept override;
        [[nodiscard]] int CurrentHP() const noexcept override { return _health.CurrentHP(); }
        [[nodiscard]] int MaxHP() const noexcept override { return _health.MaxHP(); }
        [[nodiscard]] bool IsDead() const noexcept override { return _health.IsDead(); }

    private:
        rendering::bg::BGTileManager::Id _tileset_id{}; // BG tileset to draw from
        int _tile_index{ 0 };                           // Tile index within the tileset
        Vec2 _half{ 8.0, 8.0 };                         // Half-size (used for both drawing and hit judgement)
        systems::combat::HealthComponent _health{};     // HP + per-weapon resistance
    };
}
