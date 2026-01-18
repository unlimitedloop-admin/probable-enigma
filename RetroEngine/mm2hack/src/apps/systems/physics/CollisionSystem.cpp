#include "pch.h"

#include "CollisionSystem.h"

#include <span>
#include "ICollider.h"

namespace mm2hack::apps::systems::physics
{
    void CollisionSystem::ResolveEntities(std::span<ICollider* const> cols)
    {
        const std::size_t n = cols.size();
        for (std::size_t i = 0; i < n; ++i)
        {
            ICollider* a = cols[i];
            if (!a || !a->IsCollidable() || !a->OwnerEntity().IsAlive()) continue;

            for (std::size_t j = i + 1; j < n; ++j)
            {
                ICollider* b = cols[j];
                if (!b || !b->IsCollidable() || !b->OwnerEntity().IsAlive()) continue;

                if (!_matrix.Test(a->Layer(), b->Layer())) continue;

                resolvePair_(*a, *b);
            }
        }
    }

    void CollisionSystem::resolvePair_(ICollider& a, ICollider& b)
    {
        if (!a.Bounds().intersects(b.Bounds())) return;

        // reaction (each entity decides what to do)
        a.OnEntityCollision(b.OwnerEntity());
        b.OnEntityCollision(a.OwnerEntity());
    }
}