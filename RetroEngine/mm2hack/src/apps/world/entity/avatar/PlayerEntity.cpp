#include "pch.h"

#include "PlayerEntity.h"

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "PlayerParams.h"
#include "states/BrakeRunState.h"
#include "states/HoveringState.h"
#include "states/LadderingState.h"
#include "states/LandingState.h"
#include "states/LaunchRunState.h"
#include "states/RunningState.h"
#include "states/StandingState.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::world::entity::avatar
{
    using systems::physics::CollisionLayer;

    PlayerEntity::PlayerEntity(SpriteManagerId id)
        : _id(id)
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

        refreshProbes_();

        // TODO: Need to hold this context somewhere else?
        PlayerContext cx{
            pos, vel,
            /* onGround */ onGround, /* justLanded */ false, /* prevOnGround */ onGround,
            facingLR, texture, _animeStepper,
            /* probes */ _probes, /* prelimProbes */ _probes, this->Bounds(), _terrainProbe, _ladderService
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

        // Apply updated context values
        pos = cx.pos + cx.vel;
    }

    // IRenderable
    PlayerEntity::LayerView PlayerEntity::DrawLayer() const noexcept { return LayerView::Actors; }

    void PlayerEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive()) return;
        if (!ctx.view) return;

        const auto& view = *ctx.view;
        const double worldX = pos.x;
        const double worldY = pos.y;

        const double screenX = worldX - view.camX - _half.x;
        const double screenY = worldY - view.camY - _half.y;

        // TODO: draw of DxLib
        auto& res = runtime::GameContext::GetInstance().GetResourceManager();
        res.GetSpriteManager().UseById(_id, texture, static_cast<int>(screenX), static_cast<int>(screenY));
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

    void PlayerEntity::refreshProbes_() noexcept
    {
        // Update probes based on current position and bounding box.
        double currentPosX;
        double currentPosY;
        PlayerProbes& p = _tuning.probeOffsets;
        
        // Front collision probe (ahead of the player)
        {
            currentPosX = pos.x + _half.x + (static_cast<int>(facingLR) * p.frontLineOffsetX);
            currentPosY = pos.y + _half.y;
            _probes.frontLine.topPoint =    { currentPosX, currentPosY + p.verticalTopOffsetY };
            _probes.frontLine.middlePoint = { currentPosX, currentPosY + p.verticalMidOffsetY };
            _probes.frontLine.bottomPoint = { currentPosX, currentPosY + p.verticalBtmOffsetY };
        }
        // Rear collision probe (behind the player)
        {
            currentPosX = pos.x + _half.x + (static_cast<int>(facingLR) * p.rearLineOffsetX);
            currentPosY = pos.y + _half.y;
            _probes.rearLine.topPoint =    { currentPosX, currentPosY + p.verticalTopOffsetY };
            _probes.rearLine.middlePoint = { currentPosX, currentPosY + p.verticalMidOffsetY };
            _probes.rearLine.bottomPoint = { currentPosX, currentPosY + p.verticalBtmOffsetY };
        }
        // Check on ground probe (below the player)
        {
            currentPosX = pos.x + _half.x;
            currentPosY = pos.y + _half.y + p.groundLineOffsetY;
            _probes.bottomLine.frontPoint =  { currentPosX + p.horizonFrontOffsetX,  currentPosY };
            _probes.bottomLine.middlePoint = { currentPosX + p.horizonMidOffsetX,    currentPosY };
            _probes.bottomLine.behindPoint = { currentPosX + p.horizonBehindOffsetX, currentPosY };
        }
        // Overhead probe (above the player)
        {
            currentPosX = pos.x + _half.x;
            currentPosY = pos.y + _half.y + p.overheadLineOffsetY;
            _probes.topLine.frontPoint =  { currentPosX + p.horizonFrontOffsetX,  currentPosY };
            _probes.topLine.middlePoint = { currentPosX + p.horizonMidOffsetX,    currentPosY };
            _probes.topLine.behindPoint = { currentPosX + p.horizonBehindOffsetX, currentPosY };
        }
        // Check behind BG tile probe
        {
            currentPosX = pos.x + _half.x + (static_cast<int>(facingLR) * 1);
            currentPosY = pos.y + _half.y;
            _probes.behindGround.topPoint    = { currentPosX, currentPosY + p.verticalTopOffsetY };
            _probes.behindGround.middlePoint = { currentPosX, currentPosY + p.verticalMidOffsetY };
            _probes.behindGround.bottomPoint = { currentPosX, currentPosY + p.verticalBtmOffsetY };
        }
    }
}