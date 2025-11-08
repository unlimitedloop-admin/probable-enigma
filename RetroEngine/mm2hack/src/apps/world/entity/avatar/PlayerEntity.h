//==============================================================================
// 
//  Project: mm2hack
//  PlayerEntity.h
// 
//  The one player character controlling all actions.
// 
//==============================================================================
#pragma once

#include "apps/systems/physics/ICollider.h"
#include "apps/world/entity/EntityBase.h"

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/IEntity.h"

namespace mm2hack::apps::world::entity::avatar
{
    namespace physics = systems::physics;

    // User player character entity
    class PlayerEntity final : public EntityBase, public physics::ICollider
    {
        using CollisionLayer = physics::CollisionLayer;
        using TileAttribute  = physics::TileAttribute;
        using LayerView      = systems::view::Layer;
        using RenderContext  = systems::view::RenderContext;
        using RectF          = foundation::math::RectF;
        using Vec2           = foundation::math::Vec2;
        //using IEntity        = world::entity::IEntity;

    public:
        PlayerEntity();

        // Main action updates (IUpdatable)
        void Update(double /*dt*/) override;
        // Drawing layer (IRenderable)
        LayerView DrawLayer() const noexcept override;
        // Rendering (IRenderable)
        void Render(RenderContext& /*ctx*/) override;
        // Is on alive? (IEntity)
        bool IsAlive() const noexcept override;
        // Kill (IEntity)
        void Kill() noexcept override;
        // Bounding box (ICollider)
        RectF Bounds() const override;
        // Is collidable? (ICollider)
        bool IsCollidable() const noexcept override;
        // Collision layer (ICollider)
        CollisionLayer Layer() const noexcept override;
        // Collision reactions (ICollider)
        void OnTileCollision(const Vec2& normal, TileAttribute attr) override;
        // Entity collision reaction (ICollider)
        void OnEntityCollision(IEntity& other) override;

        // Return the owner entity
        IEntity& OwnerEntity() noexcept override;
        const IEntity& OwnerEntity() const noexcept override;

        // Set collidable
        void SetCollidable(bool v) noexcept;

    private:
        const std::wstring kClassName{ L"PlayerEntity" };

        Vec2 _half{};               // Half-size of the bounding box
        bool _collidable{ true };   // Whether collision is enabled
    };
}