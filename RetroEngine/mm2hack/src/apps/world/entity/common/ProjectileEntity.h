//==============================================================================
// 
//  Project: mm2hack
//  ProjectileEntity.h
// 
//  Collidable object such as bullets or shot within the game world.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"

namespace mm2hack::apps::world::entity
{
    namespace physics = systems::physics;

    // Entity representing a projectile (bullet, shot, etc.)
    class ProjectileEntity final : public EntityBase, public physics::ICollider
    {
        using RectF          = foundation::math::RectF;
        using Vec2           = foundation::math::Vec2;
        using LayerView      = systems::view::Layer;
        using RenderContext  = systems::view::RenderContext;
        using CollisionLayer = physics::CollisionLayer;
        using TileAttribute  = physics::TileAttribute;

    public:
        explicit ProjectileEntity(bool collide_tile, bool from_player);

        // IUpdatable
        void Update(double dt) override;
        // (IRenderable)
        LayerView DrawLayer() const noexcept override;
        // Rendering (IRenderable)
        void Render(RenderContext&) override;
        // IEntity
        bool IsAlive() const noexcept override;
        // Kill (IEntity)
        void Kill() noexcept override;
        // ICollider
        RectF Bounds() const override;
        // Is collidable? (ICollider)
        bool IsCollidable() const noexcept override;
        // Collision layer (ICollider)
        CollisionLayer Layer() const noexcept override;

        // Collision reactions (ICollider)
        void OnTileCollision(const Vec2& normal, TileAttribute attr) override;
        // Entity collision reaction (ICollider)
        void OnEntityCollision(IEntity& /*other*/) override;

        IEntity& OwnerEntity() noexcept override;
        const IEntity& OwnerEntity() const noexcept override;

    private:
        const std::wstring kClassName{ L"ProjectileEntity" };

        Vec2 _half{};
        bool _collide_tile{ false };
        CollisionLayer _layer;
    };
}