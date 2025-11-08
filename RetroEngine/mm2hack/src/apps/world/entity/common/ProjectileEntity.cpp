#include "pch.h"

#include "ProjectileEntity.h"

#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"

namespace mm2hack::apps::world::entity
{
    ProjectileEntity::ProjectileEntity(bool collide_tile, bool from_player)
        : _collide_tile(collide_tile),
        _layer(from_player ? CollisionLayer::ProjectilePlayer : CollisionLayer::ProjectileEnemy)
    {
        _half = { 3.0, 3.0 };
    }

    void ProjectileEntity::Update(double dt)
    {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    }

    ProjectileEntity::LayerView ProjectileEntity::DrawLayer() const noexcept { return LayerView::Effects; }

    void ProjectileEntity::Render(RenderContext&) {}

    bool ProjectileEntity::IsAlive() const noexcept { return EntityBase::IsAlive(); }

    void ProjectileEntity::Kill() noexcept { EntityBase::Kill(); }

    ProjectileEntity::RectF ProjectileEntity::Bounds() const
    {
        return { pos.x - _half.x, pos.y - _half.y, pos.x + _half.x, pos.y + _half.y };
    }

    bool ProjectileEntity::IsCollidable() const noexcept { return true; }

    ProjectileEntity::CollisionLayer ProjectileEntity::Layer() const noexcept { return _layer; }

    void ProjectileEntity::OnTileCollision(const Vec2& normal, TileAttribute attr)
    {
        if (!_collide_tile) return;

        if (Has(attr, TileAttribute::ReflectProjectile))
        {
            // Process reflection: simple velocity inversion
            // HACK: More realistic reflection based on normal (ex. diagonally upward in the opposite direction).
            vel.x *= -1.0;
            vel.y *= -1.0;
        }
        else
        {
            Kill(); // Destroy when hitting solid tiles (or not?)
        }
    }

    void ProjectileEntity::OnEntityCollision(IEntity& /*other*/)
    {
        Kill();
    }

    IEntity& ProjectileEntity::OwnerEntity() noexcept
    {
        return *this;
    }

    const IEntity& ProjectileEntity::OwnerEntity() const noexcept
    {
        return *this;
    }
}