#include "pch.h"

#include "CollisionLayer.h"

namespace mm2hack::apps::systems::physics
{
    CollisionMatrix::CollisionMatrix()
    {
        const auto N = static_cast<size_t>(CollisionLayer::Count);
        for (size_t i = 0; i < N; ++i) for (size_t j = 0; j < N; ++j) _table[i][j] = false;

        Set(CollisionLayer::Player, CollisionLayer::Enemy, true);
        Set(CollisionLayer::Player, CollisionLayer::Trap, true);
        Set(CollisionLayer::Player, CollisionLayer::Item, true);
        Set(CollisionLayer::ProjectilePlayer, CollisionLayer::Enemy, true);
        Set(CollisionLayer::ProjectileEnemy, CollisionLayer::Player, true);
        Set(CollisionLayer::ProjectilePlayer, CollisionLayer::Trap, true);
        Set(CollisionLayer::ProjectileEnemy, CollisionLayer::Trap, true);
    }

    void CollisionMatrix::Set(const CollisionLayer a, const CollisionLayer b, const bool v) noexcept
    {
        const auto ia = static_cast<size_t>(a), ib = static_cast<size_t>(b);
        _table[ia][ib] = _table[ib][ia] = v;
    }

    [[nodiscard]] bool CollisionMatrix::Test(const CollisionLayer a, const CollisionLayer b) const noexcept
    {
        return _table[static_cast<size_t>(a)][static_cast<size_t>(b)];
    }
}