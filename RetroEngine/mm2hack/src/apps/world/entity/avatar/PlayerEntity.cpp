#include "pch.h"

#include "PlayerEntity.h"

#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "states/BrakeRunState.h"
#include "states/HoveringState.h"
#include "states/LadderingState.h"
#include "states/LandingState.h"
#include "states/LaunchRunState.h"
#include "states/RunningState.h"
#include "states/StandingState.h"

namespace mm2hack::apps::world::entity::avatar
{
    using systems::physics::CollisionLayer;

    PlayerEntity::PlayerEntity()
    {
        _half = { 16.0, 16.0 }; // Ex. 32x32
        _states[0] = std::make_unique<states::StandingState>();
        _states[1] = std::make_unique<states::RunningState>();
        _states[2] = std::make_unique<states::HoveringState>();
        _states[3] = std::make_unique<states::LaunchRunState>();
        _states[4] = std::make_unique<states::BrakeRunState>();
        _states[5] = std::make_unique<states::LadderingState>();
        _states[6] = std::make_unique<states::LandingState>();
    }

    // IUpdatable
    void PlayerEntity::Update(double dt)
    {
        if (!IsAlive()) return;

        PlayerContext cx{
            pos, vel, onGround, facingLR, texture, _animeStepper, _probes, _terrainProbe.get(), _ladderService
        };
        auto& st = FindState(_status);

        // Update state machine
        const auto next = st->Update(cx, _input, _tuning, dt);
        if (next != _status)
        {
            st->OnExit(cx);
            _status = next;
            FindState(_status)->OnEnter(cx);
        }
    }

    // IRenderable
    PlayerEntity::LayerView PlayerEntity::DrawLayer() const noexcept { return LayerView::Actors; }

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

    CollisionLayer PlayerEntity::Layer() const noexcept { return CollisionLayer::Player; }

    void PlayerEntity::OnTileCollision(const Vec2& normal, TileAttribute attr)
    {
        if (Has(attr, TileAttribute::InstantDeath)) { Kill(); return; }

        if (normal.y < 0.0) { vel.y = 0.0; onGround = true; }
        // OneWay / Ladder / Water etc...
    }

    void PlayerEntity::OnEntityCollision(IEntity& other)
    {
        (void)other;
        // TODO: Item acquisition, enemy damage, etc. goes here
    }

    IEntity& PlayerEntity::OwnerEntity() noexcept { return *this; }

    const IEntity& PlayerEntity::OwnerEntity() const noexcept { return *this; }

    void PlayerEntity::SetCollidable(bool v) noexcept { _collidable = v; }
}