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
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"
#include "AvatarStatus.h"
#include "config/SystemConfig.h"
#include "input/Jpbtn.h"
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

    PlayerEntity::PlayerEntity(SpriteManagerId id, SpriteManagerId weaponId, SpriteManagerId effectsId)
        : _id(id), _effects_id(effectsId), _half{ 16.0, 16.0 }
    {
        _attackAction = std::make_unique<states::AttackActionState>(weaponId);
        _states[0] = std::make_unique<states::StandingState>();
        _states[1] = std::make_unique<states::RunningState>();
        _states[2] = std::make_unique<states::HoveringState>();
        _states[3] = std::make_unique<states::LaunchRunState>();
        _states[4] = std::make_unique<states::BrakeRunState>();
        _states[5] = std::make_unique<states::LadderingState>();
        _states[6] = std::make_unique<states::LandingState>();

        SetTuning(PlayerTuning{});
    }

    void PlayerEntity::Update(const systems::view::ViewState* view, double dt)
    {
        using namespace systems::scrolling::atomic;
        if (!IsAlive()) return;

        PlayerContext cx{
            _id, _attackAction->Id(),
            pos, vel,
            onGround, /* justLanded */ false, /* isHitCeiling */ false, /* prevOnGround */ onGround, facingLR,
            baseTexture, /* textureAdd */ 0, _anime_stepper, /* probes */ _probes, /* prelimProbes */ _probes,
            this->Bounds(),
            _page_origin_px, _terrain_probe, _ladder_service, /* lockClimbMove */ false, _v_bounds, _scroll_rules, _scroll_page_index,
            /* pendingFixedScroll */ { _fixed_scroll_available, ScrollDir::None, 0.0 }
        };

        refreshProbes_(cx);

        // Update physical environment and select appropriate tuning.
        const PlayerEnvironment previous_environment = _environment;
        updateEnvironment_();

        if (previous_environment == PlayerEnvironment::Normal &&
            _environment == PlayerEnvironment::Underwater)
        {
            auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
            auto& audio = resource.GetAudioManager();
            audio.PlaySe(L"splash");

            constexpr int kRightSplashTexture = 0;
            constexpr int kLeftSplashTexture = 4;
            const double tile_size = static_cast<double>(config::SystemConfig::kTileSize);
            const double surface_y =
                std::floor(_probes.environment.centerPoint.y / tile_size) * tile_size;

            if (_effects_id != static_cast<SpriteManagerId>(-1))
            {
                _spawn_splash_effect = SpawnSplashEffectCommand{
                    .spawnPos = { pos.x, surface_y },
                    .spriteId = _effects_id,
                    .baseTexture = facingLR == AvatarDirection::Right
                        ? kRightSplashTexture
                        : kLeftSplashTexture
                };
            }
        }

        selectTuning_();

        const PlayerTuning& tuning = *_current_tuning;
        const bool skipPhysics = shouldSkipUnderwaterPhysics_();

        // Detect the jump edge every tick, before the skip gate below can swallow it.
        // If this tick's state Update() is about to be skipped (underwater slow-motion
        // gate), latch the press so it is not lost; kSkipInterval guarantees the gate
        // can never skip two ticks in a row, so the latch is always picked up (or
        // discarded) on the very next tick, never carried further than that.
        const bool jumpPressedNow = _input->JustPressed(JPBTN::A);
        if (jumpPressedNow && skipPhysics)
        {
            _jump_buffered = true;
        }

        // Get up and down lock on laddering (if any)
        _attackAction->PreUpdate(cx, _input, _entityContext.canSpawnProjectile);

        // 1) locomotion: state update and transition (basic behavior)
        auto& st = findState_(_status);
        AvatarStatus next = _status;
        if (!skipPhysics)
        {
            cx.jumpEdge = jumpPressedNow || _jump_buffered;
            _jump_buffered = false;

            // Execute the behavior of the current state and determine the next state.
            next = st->Update(cx, _input, tuning, dt);
        }

        // 2) attack action: update (independent of basic behavior)
        const auto act = _attackAction->PostUpdate(cx, _input, _attack_tuning, dt);

        cx.textureAdd += act.textureAdd;
        cx.lockClimbMove = cx.lockClimbMove || act.lockClimbMove;
        _rock_buster = act.rockBuster;
        _spawnProjectile = act.spawnProjectile;

        if (next != _status)
        {
            st->OnExit(cx, _input, tuning);
            _status = next;
            findState_(_status)->OnEnter(cx, _input, tuning);
        }

        if (cx.pendingFixedScroll.dir != ScrollDir::None)
        {
            requestScroll_(cx.pendingFixedScroll);
        }

        // Apply updated context values
        if (!skipPhysics)
        {
            pos = cx.pos + cx.vel;
        }
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
        findState_(_status)->TickAnimationOnly(
            ax,
            _input,
            *_current_tuning,
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

            auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
            auto& audio = resource.GetAudioManager();
            audio.PlaySe(L"onstage_thump");
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
        _normal_tuning = t;
        _underwater_tuning = MakeUnderwaterTuning(t);
        _current_tuning = &_normal_tuning;
    }

    void PlayerEntity::updateEnvironment_() noexcept
    {
        if (_terrain_probe == nullptr)
        {
            _environment = PlayerEnvironment::Normal;
            return;
        }

        const TileAttribute attr =
            _terrain_probe->AttributeAt(_probes.environment.centerPoint);

        if (Has(attr, TileAttribute::Water))
        {
            _environment = PlayerEnvironment::Underwater;
            return;
        }

        _environment = PlayerEnvironment::Normal;
    }

    void PlayerEntity::selectTuning_() noexcept
    {
        switch (_environment)
        {
        case PlayerEnvironment::Underwater:
            _current_tuning = &_underwater_tuning;
            break;

        case PlayerEnvironment::Normal:
        default:
            _current_tuning = &_normal_tuning;
            break;
        }
    }

    bool PlayerEntity::shouldSkipUnderwaterPhysics_() noexcept
    {
        if (_environment != PlayerEnvironment::Underwater)
        {
            _underwater_physics_gate.reset();
            return false;
        }

        constexpr int kSkipInterval = 5;

        return _underwater_physics_gate.step(kSkipInterval);
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

    std::optional<PlayerEntity::SpawnProjectileCommand> PlayerEntity::TakeSpawnProjectile()
    {
        return std::exchange(_spawnProjectile, std::nullopt);
    }

    std::optional<PlayerEntity::SpawnSplashEffectCommand> PlayerEntity::TakeSpawnSplashEffect()
    {
        return std::exchange(_spawn_splash_effect, std::nullopt);
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
        _probes.refreshAll(cx, _normal_tuning.probeOffsets);
    }

    void PlayerEntity::requestScroll_(FixedScrollRequest req) noexcept
    {
        if (_pending_scroll_req.has_value()) return; // keep first!
        _pending_scroll_req = std::move(req);
    }
}
