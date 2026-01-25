#include "pch.h"

#include "PlayerEntity.h"

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"
#include "AvatarStatus.h"
#include "IPlayerState.h"
#include "PlayerContext.h"
#include "PlayerParams.h"
#include "states/AttackActionState.h"
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
    using systems::scrolling::atomic::ScrollController;
    using systems::scrolling::atomic::ViewBounds;

    PlayerEntity::PlayerEntity(SpriteManagerId id)
        : _id(id), _half{ 16.0, 16.0 }
    {
        _attackAction = std::make_unique<AttackActionState>();
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
        using namespace systems::scrolling::atomic;
        if (!IsAlive()) return;

        PlayerContext cx{
            _id,
            pos, vel,
            onGround, /* justLanded */ false, /* isHitCeiling */ false, /* prevOnGround */ onGround,
            facingLR, baseTexture, /* textureAdd */ 0, _animeStepper,
            /* probes */ _probes, /* prelimProbes */ _probes, this->Bounds(),
            _pageOriginPx,
            _terrainProbe, _ladderService, false, _vBounds, _scrollRules, _scrollPageIndex,
            /* pendingFixedScroll */ { _fixedScrollAvailable, ScrollDir::None, 0.0 }
        };

        _spawnProjectile.reset();
        _spawnProjectile = _attackAction->Update(cx, _input, _attack_tuning, _entityContext.canSpawnProjectile, dt);

        refreshProbes_(cx);
        auto& st = FindState(_status);
        const auto next = st->Update(cx, _input, _tuning, dt);  // Update state machine

        if (next != _status)
        {
            st->OnExit(cx, _input, _tuning);
            _status = next;
            FindState(_status)->OnEnter(cx, _input, _tuning);
        }

        if (cx.pendingFixedScroll.dir != ScrollDir::None)
        {
            requestScroll_(cx.pendingFixedScroll);
        }

        // Apply updated context values
        pos = cx.pos + cx.vel;
        texture = cx.texture;
        attackTexture = cx.textureAdd;
        composeFinalTexture_();

        // Shift the avatar's position (coordinates) to match the terrain. This is mainly done against the terrain underfoot.
        // after updating pos with vel.
        refreshProbes_(cx);

        if (cx.justLanded)
        {
            const auto fix = cx.terrain->ResolveOverlapX(cx.probes, config::SystemConfig::kEpsilon);    // Repenetration fix on X-axis.
            if (fix.hit && fix.pushX != 0.0)
            {
                pos.x += fix.pushX;
                refreshProbes_(cx);
            }
        }

        if (cx.isHitCeiling)
        {
            vel.y = 0.0;
        }
    }

    void PlayerEntity::TickAnimation(double dt)
    {
        if (!IsAlive()) return;

        int text_add = 0;
        AnimeContext ax{ _animeStepper, facingLR, baseTexture, text_add };
        _attackAction->TickAnimationOnly(ax, _attack_tuning, dt);
        FindState(_status)->TickAnimationOnly(ax, _input, _tuning, dt);
        
        attackTexture = text_add;
        composeFinalTexture_();
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

        const double screenX = worldX - view.viewWorldX - _half.x;
        const double screenY = worldY - view.viewWorldY - _half.y;

        auto& res = runtime::GameContext::GetInstance().GetResourceManager();
        res.GetSpriteManager().UseById(_id, texture, static_cast<int>(screenX), static_cast<int>(screenY));

        if (_attackAction->IsAttacking())
        {
            // REVIEW: Hardcoded offsets!
            const int weaponTexture = facingLR == AvatarDirection::Left ? 50 : 10;
            auto weaponOffset = _attackAction->GetRockBusterOffset(texture, facingLR);
            const int weaponOffsetX = static_cast<int>(weaponOffset.x);
            const int weaponOffsetY = static_cast<int>(weaponOffset.y);
            res.GetSpriteManager().UseById(_id, weaponTexture, static_cast<int>(screenX) + weaponOffsetX, static_cast<int>(screenY) + weaponOffsetY);
        }
    }

    bool PlayerEntity::IsAlive() const noexcept { return EntityBase::IsAlive(); }

    void PlayerEntity::Kill() noexcept { EntityBase::Kill(); }

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
        // OneWay / Laddering / Water etc...
    }

    void PlayerEntity::OnEntityCollision(IEntity& other)
    {
        (void)other;
        // TODO: Item acquisition, enemy damage, etc. goes here
    }

    IEntity& PlayerEntity::OwnerEntity() noexcept { return *this; }

    const IEntity& PlayerEntity::OwnerEntity() const noexcept { return *this; }

    void PlayerEntity::SetCollidable(bool v) noexcept { _collidable = v; }

    void PlayerEntity::SetViewBounds(const systems::scrolling::atomic::ViewBounds& b) noexcept
    {
        _vBounds.leftX = b.leftX;
        _vBounds.rightX = b.rightX;
        _vBounds.topY = b.topY;
        _vBounds.bottomY = b.bottomY;
    }

    void PlayerEntity::SetScrollContext(const IScrollRuleProvider* rules, std::size_t pageIndex)
    {
        _scrollRules = rules;
        _scrollPageIndex = pageIndex;
    }

    [[nodiscard]] std::optional<FixedScrollRequest> PlayerEntity::ConsumeScrollRequest() noexcept
    {
        if (!_pendingScrollReq.has_value()) return std::nullopt;
        auto out = _pendingScrollReq;
        _pendingScrollReq.reset();
        return out;
    }

    std::optional<PlayerEntity::SpawnProjectileCommand> PlayerEntity::TakeSpawnProjectile()
    {
        return std::exchange(_spawnProjectile, std::nullopt);
    }

    void PlayerEntity::composeFinalTexture_() noexcept
    {
        int t = baseTexture + attackTexture;

        // Facing offset is applied exactly once here.
        if (facingLR == AvatarDirection::Left)
        {
            t += static_cast<int>(AvatarAnimation::ToTheLeft);
        }
        else
        {
            t += static_cast<int>(AvatarAnimation::ToTheRight);
        }

        texture = t;
    }

    void PlayerEntity::refreshProbes_(PlayerContext& cx) noexcept
    {
        _probes.refreshAll(cx, _tuning.probeOffsets);
    }

    void PlayerEntity::requestScroll_(FixedScrollRequest req) noexcept
    {
        if (_pendingScrollReq.has_value()) return; // keep first!
        _pendingScrollReq = std::move(req);
    }
}