#include "pch.h"

#include "PlayerEntity.h"

#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"

namespace mm2hack::apps::world::entity::avatar
{
    PlayerEntity::PlayerEntity()
    {
        _half = { 8.0, 12.0 }; // Ex. 16x24
    }

    // IUpdatable
    void PlayerEntity::Update(double /*dt*/)
    {
        // TODO: Action algorithm
    }

    // IRenderable
    PlayerEntity::LayerView PlayerEntity::DrawLayer() const noexcept
    {
        return LayerView::Actors;
    }
    void PlayerEntity::Render(RenderContext& /*ctx*/)
    {
        // TODO: draw of DxLib
    }

    // IEntity
    bool PlayerEntity::IsAlive() const noexcept { return EntityBase::IsAlive(); }

    void PlayerEntity::Kill() noexcept { EntityBase::Kill(); }

    // ICollider
    PlayerEntity::RectF PlayerEntity::Bounds() const
    {
        return { pos.x - _half.x, pos.y - _half.y,
                 pos.x + _half.x, pos.y + _half.y };
    }

    bool PlayerEntity::IsCollidable() const noexcept { return _collidable; }

    physics::CollisionLayer PlayerEntity::Layer() const noexcept
    {
        return physics::CollisionLayer::Player;
    }

    void PlayerEntity::OnTileCollision(const Vec2& normal, physics::TileAttribute attr)
    {
        if (physics::Has(attr, physics::TileAttribute::InstantDeath))
        {
            Kill();
            return;
        }
        if (normal.y < 0.0) vel.y = 0.0; // On floor
        // OneWay / Ladder / Water etc...
    }

    void PlayerEntity::OnEntityCollision(IEntity& other)
    {
        (void)other;
        // TODO: Item acquisition, enemy damage, etc. goes here
    }

    IEntity& PlayerEntity::OwnerEntity() noexcept
    {
        return *this;
    }

    const IEntity& PlayerEntity::OwnerEntity() const noexcept
    {
        return *this;
    }

    void PlayerEntity::SetCollidable(bool v) noexcept { _collidable = v; }
}