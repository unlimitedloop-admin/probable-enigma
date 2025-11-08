//==============================================================================
// 
//  Project: mm2hack
//  ICollider.h
// 
//  TODO: Add description.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/IEntity.h"
#include "CollisionLayer.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    using foundation::math::RectF;
    using foundation::math::Vec2;
    using world::entity::IEntity;

    // Interface for collidable entities
    struct ICollider
    {
        virtual ~ICollider() = default;

        // Return the owner entity
        virtual IEntity& OwnerEntity() noexcept = 0;
        virtual const IEntity& OwnerEntity() const noexcept = 0;

        // Axis-aligned bounding box: Center-based is also OK (Entity's responsibility)
        virtual RectF Bounds() const = 0;

        // Enabled/Disabled (e.g., switch traps, invincibility frames)
        virtual bool IsCollidable() const noexcept = 0;

        // Collision layer
        virtual CollisionLayer Layer() const noexcept = 0;

        // Reactions
        virtual void OnTileCollision(const Vec2& normal, TileAttribute attr) = 0;
        virtual void OnEntityCollision(IEntity& other) = 0;
    };
}