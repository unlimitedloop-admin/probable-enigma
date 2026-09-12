//==============================================================================
// 
//  Project: mm2hack
//  CollisionLayer.h
// 
//  Interface for all collision layers.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>

namespace mm2hack::apps::systems::physics
{
    // Collision layers for entities
    enum class CollisionLayer : std::uint8_t
    {
        Player = 0,
        Enemy,
        Item,
        Trap,
        ProjectilePlayer,
        ProjectileEnemy,
        Count
    };

    // Collision matrix for collision layers (sprites/entities)
    struct CollisionMatrix
    {
        std::array<std::array<bool, static_cast<std::size_t>(CollisionLayer::Count)>,
            static_cast<std::size_t>(CollisionLayer::Count)> _table{};

        CollisionMatrix();
        ~CollisionMatrix() = default;

        void Set(CollisionLayer a, CollisionLayer b, bool v) noexcept;
        [[nodiscard]] bool Test(CollisionLayer a, CollisionLayer b) const noexcept;
    };
}