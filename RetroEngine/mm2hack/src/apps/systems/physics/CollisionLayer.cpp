#include "pch.h"

#include "CollisionLayer.h"

namespace mm2hack::apps::systems::physics
{
    CollisionMatrix::CollisionMatrix()
    {
        const auto N = static_cast<std::size_t>(CollisionLayer::Count);
        for (std::size_t i = 0; i < N; ++i) for (std::size_t j = 0; j < N; ++j) _table[i][j] = false;

        // Player vs ...
        Set(CollisionLayer::Player, CollisionLayer::Enemy, true);
        Set(CollisionLayer::Player, CollisionLayer::Item, true);
        Set(CollisionLayer::Player, CollisionLayer::Trap, true);
        Set(CollisionLayer::Player, CollisionLayer::ProjectileEnemy, true);

        // Enemy vs ...
        Set(CollisionLayer::Enemy, CollisionLayer::ProjectilePlayer, true);

        // Projectiles (optional)
        Set(CollisionLayer::ProjectilePlayer, CollisionLayer::Trap, true);
    }

    void CollisionMatrix::Set(CollisionLayer a, CollisionLayer b, bool v) noexcept
    {
        const auto ia = static_cast<std::size_t>(a), ib = static_cast<std::size_t>(b);
        _table[ia][ib] = _table[ib][ia] = v;
    }

    [[nodiscard]] bool CollisionMatrix::Test(CollisionLayer a, CollisionLayer b) const noexcept
    {
        return _table[static_cast<std::size_t>(a)][static_cast<std::size_t>(b)];
    }
}