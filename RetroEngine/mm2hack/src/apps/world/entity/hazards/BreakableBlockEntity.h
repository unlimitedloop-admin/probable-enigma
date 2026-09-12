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
#include "apps/rendering/sprite/SpriteManager.h"
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
        std::int32_t base_texture{};
        std::int32_t max_hp{ 1 };
        std::int32_t hp{ 1 };
        foundation::math::Vec2 half_size{ 8.0, 8.0 };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    // Entity representing a destructible block (verification object for hit detection).
    class BreakableBlockEntity final : public EntityBase, public systems::physics::ICollider
    {
        // NOTE: no `Layer` alias here -- it would collide with ICollider::Layer() below.
        using RectF = foundation::math::RectF;
        using Vec2 = foundation::math::Vec2;
        using CollisionLayer = systems::physics::CollisionLayer;
        using TileAttribute = systems::physics::TileAttribute;

    public:
        static constexpr std::uint16_t kStateVersion{ 1 };

        // Fresh placement (level/test spawn)
        BreakableBlockEntity(
            Vec2 spawn_pos,
            rendering::sprite::SpriteManager::Id sprite_id,
            int base_texture,
            int max_hp,
            Vec2 half_size);
        // Reconstruction from saved state
        BreakableBlockEntity(
            const BreakableBlockEntityState& state,
            rendering::sprite::SpriteManager::Id sprite_id);

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

    private:
        rendering::sprite::SpriteManager::Id _id{};    // Object sprite id
        int _base_texture{ 0 };                        // Base texture index
        Vec2 _half{ 8.0, 8.0 };                        // Half-size (used for both drawing and hit judgement)
        int _max_hp{ 1 };                              // Max HP (kept for save validity / future cracked-look stages)
        int _hp{ 1 };                                  // Remaining HP
    };
}
