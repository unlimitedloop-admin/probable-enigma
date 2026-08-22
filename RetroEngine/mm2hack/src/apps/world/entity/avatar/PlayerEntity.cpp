#include "pch.h"

#include "PlayerEntity.h"

#include <cmath>
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnSplashEffectCommand.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"
#include "AvatarStatus.h"
#include "config/SystemConfig.h"
#include "input/Jpbtn.h"
#include "PlayerContext.h"
#include "PlayerEnvironmentController.h"
#include "PlayerFrameOutput.h"
#include "PlayerParams.h"
#include "states/AttackActionState.h"

namespace mm2hack::apps::world::entity::avatar
{
    using systems::physics::CollisionLayer;
    using systems::scrolling::atomic::ScrollController;
    using systems::scrolling::atomic::ViewBounds;

    PlayerEntity::PlayerEntity(SpriteManagerId id, SpriteManagerId weaponId, SpriteManagerId effectsId)
        : _id(id), _effects_id(effectsId), _half{ 16.0, 16.0 }
    {
        _attackAction = std::make_unique<states::AttackActionState>(weaponId);
        SetTuning(PlayerTuning{});
    }

    void PlayerEntity::Update(const systems::view::ViewState* view, double dt)
    {
        (void)view;
        if (!IsAlive()) return;

        PlayerContext cx = makeContext_();
        refreshProbes_(cx);
        const PlayerEnvironmentUpdate environment = processEnvironment_();

        const PlayerTuning& tuning = _environment_controller.CurrentTuning();

        updateActions_(cx, tuning, environment.skipPhysics, dt);
        applyContext_(cx, environment.skipPhysics);
        resolvePostMovement_(cx);
    }

    PlayerContext PlayerEntity::makeContext_()
    {
        return PlayerContext{
            _id, _attackAction->Id(),
            pos, vel,
            onGround, /* justLanded */ false, /* isHitCeiling */ false, /* prevOnGround */ onGround, facingLR,
            baseTexture, /* textureAdd */ 0, _anime_stepper, /* probes */ _probes, /* prelimProbes */ _probes,
            Bounds(),
            _page_origin_px, _terrain_probe, _ladder_service, /* lockClimbMove */ false, _v_bounds, _scroll_rules, _scroll_page_index,
            /* pendingFixedScroll */ { _fixed_scroll_available, ScrollDir::None, 0.0 },
            /* jumpEdge */ false, _frame_output
        };
    }

    PlayerEnvironmentUpdate PlayerEntity::processEnvironment_()
    {
        const PlayerEnvironmentUpdate environment =
            _environment_controller.Update(_terrain_probe, _probes.environment.centerPoint);

        if (environment.EnteredWater())
        {
            _frame_output.PushEvent(PlayerEventType::EnteredWater);

            constexpr int kRightSplashTexture = 0;
            constexpr int kLeftSplashTexture = 4;
            const double tile_size = static_cast<double>(config::SystemConfig::kTileSize);
            const double surface_y =
                std::floor(_probes.environment.centerPoint.y / tile_size) * tile_size;

            if (_effects_id != static_cast<SpriteManagerId>(-1))
            {
                _frame_output.splashEffect = common::SpawnSplashEffectCommand{
                    .spawnPos = { pos.x, surface_y },
                    .spriteId = _effects_id,
                    .baseTexture = facingLR == AvatarDirection::Right
                        ? kRightSplashTexture
                        : kLeftSplashTexture
                };
            }
        }

        return environment;
    }

    void PlayerEntity::updateActions_(PlayerContext& cx, const PlayerTuning& tuning, bool skipPhysics, double dt)
    {
        const bool jump_pressed_now = _input->JustPressed(JPBTN::A);
        if (jump_pressed_now && skipPhysics)
        {
            _jump_buffered = true;
        }

        _attackAction->PreUpdate(cx, _input, _entityContext.canSpawnProjectile);

        if (!skipPhysics)
        {
            cx.jumpEdge = jump_pressed_now || _jump_buffered;
            _jump_buffered = false;
            _state_machine.Update(cx, _input, tuning, dt);
        }

        auto action = _attackAction->PostUpdate(cx, _input, _attack_tuning, dt);
        cx.textureAdd += action.textureAdd;
        cx.lockClimbMove = cx.lockClimbMove || action.lockClimbMove;
        _rock_buster = action.rockBuster;
        if (action.spawnProjectile.has_value())
        {
            _frame_output.projectile = std::move(action.spawnProjectile);
        }

        _state_machine.CommitTransition(cx, _input, tuning);

        if (cx.pendingFixedScroll.dir != ScrollDir::None)
        {
            requestScroll_(cx.pendingFixedScroll);
        }
    }

    void PlayerEntity::applyContext_(const PlayerContext& cx, bool skipPhysics)
    {
        if (!skipPhysics)
        {
            pos = cx.pos + cx.vel;
        }

        baseTexture = cx.basePose;
        attackTexture = cx.textureAdd;
        facingLR = cx.facingLR;
        composeFinalTexture_();
    }

    void PlayerEntity::resolvePostMovement_(PlayerContext& cx)
    {
        refreshProbes_(cx);

        if (cx.justLanded)
        {
            const auto fix = cx.terrain->ResolveOverlapX(cx.probes, config::SystemConfig::kEpsilon);
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
        auto screenPos = toScreenPos(pos);
        if (_intro_states.active)
        {
            // During intro drop, override the texture to the intro drop texture
            screenPos += _intro_states.offsetPos;
        }

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
        AnimeContext ax{ _anime_stepper, facingLR, baseTexture, texture_add };
        _attackAction->TickAnimationOnly(ax, _attack_tuning, dt, _rock_buster);
        _state_machine.TickAnimation(
            ax,
            _input,
            _environment_controller.CurrentTuning(),
            dt);

        attackTexture = texture_add;
        composeFinalTexture_();
    }

    void PlayerEntity::BeginIntroDrop()
    {
        _intro_states.offsetPos = Vec2{ -1.0, -6.0 };  // Slight horizontal offset while dropping sprite set.
        _intro_states.destPos.y = pos.y;
        // Start above the screen and drop down to the current position.

        pos.y = _v_bounds.topY - _half.y - 16.0; // Start 16px above the top view bound (adjust as needed)
        baseTexture = static_cast<int>(STile::IntroDropA);  // Intro drop texture index
        _intro_states.active = true;

        composeFinalTexture_();
    }

    void PlayerEntity::UpdateIntroAnimation(double dt)
    {
        if (!_intro_states.active)
            return;

        switch (_intro_states.phase)
        {
        case IntroPhase::Falling:
            UpdateIntroFalling(dt);
            break;

        case IntroPhase::Landing:
            UpdateIntroLanding(dt);
            break;

        case IntroPhase::Done:
            break;
        }

        composeFinalTexture_();
    }

    void PlayerEntity::UpdateIntroFalling(double dt)
    {
        _intro_states.timer += dt;

        const double duration = _intro_states.dropDuration;

        double t = _intro_states.timer / duration;
        if (t > 1.0)
        {
            t = 1.0;
        }

        // Ease-in (quadratic)
        const double eased = t * t;

        const double startY = _intro_states.offsetPos.y;
        const double destY = _intro_states.destPos.y;

        pos.y = startY + (destY - startY) * eased;

        if (t >= 1.0)
        {
            pos.y = destY;
            _intro_states.phase = IntroPhase::Landing;
            _intro_states.timer = 0.0;

            _frame_output.PushEvent(PlayerEventType::IntroLanded);
        }
    }

    void PlayerEntity::UpdateIntroLanding(double dt)
    {
        _intro_states.timer += dt;

        double accumulated = 0.0;

        for (const auto& frame : kLandingFrames)
        {
            accumulated += frame.duration;

            if (_intro_states.timer < accumulated)
            {
                baseTexture = static_cast<int>(frame.tile);
                return;
            }
        }

        // Animation finished
        baseTexture = static_cast<int>(STile::StandingA);
        _intro_states.phase = IntroPhase::Done;
        _intro_states.active = false;
    }

    bool PlayerEntity::IsIntroFinished() const noexcept
    {
        return !_intro_states.active;
    }

    void PlayerEntity::SetCollidable(bool v) noexcept { _collidable = v; }

    void PlayerEntity::SetTuning(const PlayerTuning& t)
    {
        _environment_controller.SetTuning(t);
    }

    void PlayerEntity::SetViewBounds(const systems::scrolling::atomic::ViewBounds& b) noexcept
    {
        _v_bounds.leftX = b.leftX;
        _v_bounds.rightX = b.rightX;
        _v_bounds.topY = b.topY;
        _v_bounds.bottomY = b.bottomY;
    }

    void PlayerEntity::SetScrollContext(const IScrollRuleProvider* rules, std::size_t pageIndex)
    {
        _scroll_rules = rules;
        _scroll_page_index = pageIndex;
    }

    [[nodiscard]] std::optional<FixedScrollRequest> PlayerEntity::ConsumeScrollRequest() noexcept
    {
        if (!_pending_scroll_req.has_value()) return std::nullopt;
        auto out = _pending_scroll_req;
        _pending_scroll_req.reset();
        return out;
    }

    PlayerFrameOutput PlayerEntity::TakeFrameOutput() noexcept
    {
        return std::exchange(_frame_output, PlayerFrameOutput{});
    }

    void PlayerEntity::composeFinalTexture_() noexcept
    {
        int t = baseTexture + attackTexture;

        // Facing offset is applied exactly once here.
        if (facingLR == AvatarDirection::Left)
        {
            t += static_cast<int>(STile::ToTheLeft);
        }
        else
        {
            t += static_cast<int>(STile::ToTheRight);
        }

        texture = t;
    }

    void PlayerEntity::refreshProbes_(PlayerContext& cx) noexcept
    {
        _probes.refreshAll(cx, _environment_controller.ProbeOffsets());
    }

    void PlayerEntity::requestScroll_(FixedScrollRequest req) noexcept
    {
        if (_pending_scroll_req.has_value()) return; // keep first!
        _pending_scroll_req = std::move(req);
    }
}
