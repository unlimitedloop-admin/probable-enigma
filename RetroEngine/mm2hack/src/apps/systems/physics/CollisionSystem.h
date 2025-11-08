//==============================================================================
// 
//  Project: mm2hack
//  CollisionSystem.h
// 
//  Collision detection control for tile map and entities.
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include "CollisionLayer.h"

namespace mm2hack::apps::systems::physics
{
    struct ICollider;
    struct ITileMapProvider;
}

namespace mm2hack::apps::systems::physics
{
    // Collision handling system
    class CollisionSystem
    {
    public:
        explicit CollisionSystem(const ITileMapProvider* provider) : _provider(provider) {}
        ~CollisionSystem() = default;

        // Set collision matrix
        void SetMatrix(const CollisionMatrix& m) { _matrix = m; }
        // Tile collision: call only the necessary entities
        void ResolveTile(ICollider& col);
        // Entity-to-entity
        void ResolveEntities(std::vector<ICollider*>& cols);

    private:
        const std::wstring kClassName{ L"CollisionSystem" };

        const ITileMapProvider* _provider{};            // Tile map provider
        CollisionMatrix _matrix{};                      // Collision matrix

        void resolvePair_(ICollider& a, ICollider& b);  // Resolve collision between two entities
    };
}