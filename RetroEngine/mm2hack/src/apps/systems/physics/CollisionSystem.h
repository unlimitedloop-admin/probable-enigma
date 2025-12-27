//==============================================================================
// 
//  Project: mm2hack
//  CollisionSystem.h
// 
//  Collision detection control for tile map and entities.
// 
//==============================================================================
#pragma once

#include <span>
#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "CollisionLayer.h"

namespace mm2hack::apps::systems::physics
{
    struct ICollider;
}

namespace mm2hack::apps::systems::physics
{
    // Collision handling system
    class CollisionSystem
    {
    public:
        explicit CollisionSystem() {}
        ~CollisionSystem() = default;

        // Set collision matrix
        void SetMatrix(const CollisionMatrix& m) { _matrix = m; }
        // Entity-to-entity
        void ResolveEntities(std::span<ICollider* const> cols);

    private:
        void resolvePair_(ICollider& a, ICollider& b);  // Resolve collision between two entities

    private:
        const std::wstring kClassName{ L"CollisionSystem" };

        CollisionMatrix _matrix{};      // Collision matrix
    };
}