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

    PlayerEntity::PlayerEntity(SpriteManagerId id, SpriteManagerId weaponId)
        : _id(id), _half{ 16.0, 16.0 }
    {
        _attackAction = std::make_unique<states::AttackActionState>(weaponId);
        _states[0] = std::make_unique<states::StandingState>();
        _states[1] = std::make_unique<states::RunningState>();
        _states[2] = std::make_unique<states::HoveringState>();
        _states[3] = std::make_unique<states::LaunchRunState>();
        _states[4] = std::make_unique<states::BrakeRunState>();
        _states[5] = std::make_unique<states::LadderingState>();
        _states[6] = std::make_unique<states::LandingState>();
    }

    void PlayerEntity::Update(const systems::view::ViewState* view, double dt)
    {
        using namespace systems::scrolling::atomic;
        if (!IsAlive()) return;

        PlayerContext cx{
            _id, _attackAction->Id(),
            pos, vel,
            onGround, /* justLanded */ false, /* isHitCeiling */ false, /* prevOnGround */ onGround, facingLR,
            baseTexture, /* textureAdd */ 0, _animeStepper, /* probes */ _probes, /* prelimProbes */ _probes,
            this->Bounds(),
            _pageOriginPx, _terrainProbe, _ladderService, /* lockClimbMove */ false, _vBounds, _scrollRules, _scrollPageIndex,
            /* pendingFixedScroll */ { _fixedScrollAvailable, ScrollDir::None, 0.0 }
        };

        refreshProbes_(cx);

        // Get up and down lock on laddering (if any)
        _attackAction->PreUpdate(cx, _input, _entityContext.canSpawnProjectile);

        // 1) locomotion: state update and transition (basic behavior)
        auto& st = FindState(_status);
        const auto next = st->Update(cx, _input, _tuning, dt);

        // 2) attack action: update (independent of basic behavior)
        const auto act = _attackAction->PostUpdate(cx, _input, _attack_tuning, dt);

        cx.textureAdd += act.textureAdd;
        cx.lockClimbMove = cx.lockClimbMove || act.lockClimbMove;
        _rock_buster = act.rockBuster;
        _spawnProjectile = act.spawnProjectile;

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
        baseTexture = cx.basePose;
        attackTexture = cx.textureAdd;
        facingLR = cx.facingLR;
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

    PlayerEntity::LayerView PlayerEntity::DrawLayer() const noexcept { return LayerView::Actors; }

    void PlayerEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive()) return;
        if (!ctx.view) return;

        const auto toScreenPos = [&](const Vec2& worldPos) -> Vec2
        {
            return {
                worldPos.x - ctx.view->viewWorldX - _half.x,
                worldPos.y - ctx.view->viewWorldY - _half.y
            };
        };

        auto& res = runtime::GameContext::GetInstance().GetResourceManager();
        
        // Draw player sprite
        const auto screenPos = toScreenPos(pos);
        res.GetSpriteManager().UseById(_id, texture, static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));

        // Draw rock buster arm if visible
        if (_rock_buster.visible)
        {
            const Vec2 armWorldPos = pos + _rock_buster.offset;
            const auto armScreenPos = toScreenPos(armWorldPos);
            res.GetSpriteManager().UseById(_id, _rock_buster.armTexture, static_cast<int>(armScreenPos.x), static_cast<int>(armScreenPos.y));
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

    void PlayerEntity::TickAnimation(double dt)
    {
        if (!IsAlive()) return;

        int texture_add = 0;
        AnimeContext ax{ _animeStepper, facingLR, baseTexture, texture_add };
        _attackAction->TickAnimationOnly(ax, _attack_tuning, dt, _rock_buster);
        FindState(_status)->TickAnimationOnly(ax, _input, _tuning, dt);

        attackTexture = texture_add;
        composeFinalTexture_();
    }

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