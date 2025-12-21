#include "pch.h"

#include "CollisionSystem.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "ICollider.h"
#include "ITileMapProvider.h"
#include "TileAttribute.h"

namespace mfm = mm2hack::apps::foundation::math;

namespace mm2hack::apps::systems::physics
{
    // TODO: translate to English comments later :)
    void CollisionSystem::ResolveTile(ICollider& col)
    {
        if (!_provider || !col.IsCollidable()) return;

        const auto b = col.Bounds();
        const int ts = _provider->TileSize();

        const int l = static_cast<int>(b.left()) / ts;
        const int r = static_cast<int>(b.right()) / ts;
        const int t = static_cast<int>(b.top()) / ts;
        const int bt = static_cast<int>(b.bottom()) / ts;

        mfm::Vec2 normal{ 0.0, 0.0 };

        for (int ty = t; ty <= bt; ++ty)
        {
            for (int tx = l; tx <= r; ++tx)
            {
                const auto attr = _provider->SampleTileAttribute(tx, ty);
                if (Has(attr, TileAttribute::InstantDeath))
                {
                    col.OnTileCollision({ 0.0, 0.0 }, attr); // Death/reset, etc. are handled by the entity
                    return;
                }
                if (Has(attr, TileAttribute::Solid))
                {
                    // Delegate undo on moving when approaching a wall to the entity. Notify with normal vector.
                    normal = { 0.0, -1.0 };
                    col.OnTileCollision(normal, attr);
                }
                else if (Has(attr, TileAttribute::ReflectProjectile))
                {
                    // Reflect projectile wall. Notify with dummy normal.
                    col.OnTileCollision({ -1.0, -1.0 }, attr);
                }
                // TODO: Laddering / OneWay / Water / Damage 
            }
        }
    }

    void CollisionSystem::ResolveEntities(std::vector<ICollider*>& cols)
    {
        const size_t n = cols.size();
        for (size_t i = 0; i + 1 < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                if (cols[i] && cols[j]) resolvePair_(*cols[i], *cols[j]);
            }
        }
    }

    void CollisionSystem::resolvePair_(ICollider& a, ICollider& b)
    {
        if (!a.IsCollidable() && !b.IsCollidable()) return;
        if (!_matrix.Test(a.Layer(), b.Layer()))   return;
        if (!mfm::overlap(a.Bounds(), b.Bounds())) return;

        auto& ea = a.OwnerEntity();
        auto& eb = b.OwnerEntity();

        a.OnEntityCollision(eb);
        b.OnEntityCollision(ea);
    }
}