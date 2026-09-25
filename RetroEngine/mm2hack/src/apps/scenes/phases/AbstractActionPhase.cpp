#include "pch.h"

#include "AbstractActionPhase.h"

#include <cmath>
#include <cstdlib>
#include <vector>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/resources/parameters/Parameters.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/systems/audio/ApuVoice.h"
#include "apps/systems/audio/AudioManager.h"
#include "apps/systems/audio/SeTransportState.h"
#include "apps/systems/audio/SoundChannel.h"
#include "apps/systems/combat/IDamageable.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "apps/world/entity/avatar/PlayerFrameOutput.h"
#include "apps/world/entity/common/SpawnChargeEffectCommand.h"
#include "apps/world/entity/common/SpawnMissBubbleEffectCommand.h"
#include "apps/world/entity/common/SpawnSlidingDustEffectCommand.h"
#include "apps/world/entity/common/SpawnSmallExplosionEffectCommand.h"
#include "apps/world/entity/effects/ChargeEffectEntity.h"
#include "apps/world/entity/effects/MissBubbleEffectEntity.h"
#include "apps/world/entity/effects/ProjectileEntity.h"
#include "apps/world/entity/effects/SlidingDustEffectEntity.h"
#include "apps/world/entity/effects/SmallExplosionEffectEntity.h"
#include "apps/world/entity/effects/SplashEffectEntity.h"
#include "apps/world/entity/enemy/EnemyEntity.h"
#include "apps/world/entity/EntityManager.h"
#include "apps/world/entity/EntityStateFactory.h"
#include "config/ConfigUIManager.h"
#include "core/overlay/DebugHud.h"
#include "core/save/StateIO.h"
#include "input/Jpbtn.h"
#include "IPhaseHost.h"
#include "IStageScript.h"
#include "PhaseResult.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::scenes::phases
{
    namespace
    {
        const std::wstring kChargeSeName{ L"rock_buster_charge" };

        struct ChargeParticleStep final
        {
            int offset_x;
            int base_texture;
        };

        // Cosmetic variation must be independent of wall-clock time and mutable
        // random-generator state so save/load and input replay follow the same path.
        constexpr std::array kChargeParticlePattern{
            ChargeParticleStep{ -12, 0 },
            ChargeParticleStep{   7, 8 },
            ChargeParticleStep{  -3, 0 },
            ChargeParticleStep{  11, 0 },
            ChargeParticleStep{  -8, 8 },
            ChargeParticleStep{   2, 8 },
            ChargeParticleStep{  12, 0 },
            ChargeParticleStep{  -5, 8 },
            ChargeParticleStep{   5, 0 },
            ChargeParticleStep{ -10, 0 },
            ChargeParticleStep{   0, 8 },
            ChargeParticleStep{   9, 8 },
        };

        // ======== Miss sequence ========
        // Ticks from the miss until the restart's fade-out begins, per cause.
        // A pit miss has little to watch (the bubbles start off-screen), so it
        // cuts to the restart sooner.
        constexpr int kMissOutOfVitalityFadeOutDelayFrames = 0xC8;  // 200
        constexpr int kMissFellIntoPitFadeOutDelayFrames = 0x5A;    // 90

        [[nodiscard]] constexpr int MissFadeOutDelayFrames(MissCause cause) noexcept
        {
            switch (cause)
            {
            case MissCause::FellIntoPit:
                return kMissFellIntoPitFadeOutDelayFrames;
            case MissCause::OutOfVitality:
            default:
                return kMissOutOfVitalityFadeOutDelayFrames;
            }
        }
        // Two rings of 8 bubbles each, all leaving the player's position at
        // once: the outer ring fast, the inner one at half its speed (px/tick).
        constexpr double kMissOuterRingSpeed = 2.0;
        constexpr double kMissInnerRingSpeed = 1.0;
        constexpr double kDiagonal = 0.70710678118654752; // 1/sqrt(2)
        constexpr std::array kMissBubbleDirections{
            foundation::math::Vec2{  1.0,        0.0       },
            foundation::math::Vec2{  kDiagonal, -kDiagonal },
            foundation::math::Vec2{  0.0,       -1.0       },
            foundation::math::Vec2{ -kDiagonal, -kDiagonal },
            foundation::math::Vec2{ -1.0,        0.0       },
            foundation::math::Vec2{ -kDiagonal,  kDiagonal },
            foundation::math::Vec2{  0.0,        1.0       },
            foundation::math::Vec2{  kDiagonal,  kDiagonal },
        };
    }

    bool AbstractActionPhaseState::Save(core::save::StateWriter& writer) const
    {
        if (!IsValid())
        {
            return false;
        }

        return
            writer.WriteU8(static_cast<std::uint8_t>(phase)) &&
            writer.WriteU8(static_cast<std::uint8_t>(intro_step)) &&
            writer.WriteF64(intro_timer) &&
            writer.WriteBool(entered) &&
            writer.WriteBool(operate) &&
            writer.WriteF64(player_previous_position.x) &&
            writer.WriteF64(player_previous_position.y) &&
            ready_ui.Save(writer) &&
            scroll.Save(writer) &&
            writer.WriteU64(enemy_attack_pattern_counter);
    }

    bool AbstractActionPhaseState::Load(core::save::StateReader& reader)
    {
        AbstractActionPhaseState loaded{};
        std::uint8_t encoded_phase{};
        std::uint8_t encoded_intro_step{};
        if (!reader.ReadU8(encoded_phase) ||
            !reader.ReadU8(encoded_intro_step) ||
            !reader.ReadF64(loaded.intro_timer) ||
            !reader.ReadBool(loaded.entered) ||
            !reader.ReadBool(loaded.operate) ||
            !reader.ReadF64(loaded.player_previous_position.x) ||
            !reader.ReadF64(loaded.player_previous_position.y) ||
            !loaded.ready_ui.Load(reader) ||
            !loaded.scroll.Load(reader) ||
            !reader.ReadU64(loaded.enemy_attack_pattern_counter))
        {
            return false;
        }

        loaded.phase = static_cast<ActionPhaseState>(encoded_phase);
        loaded.intro_step = static_cast<ActionIntroStep>(encoded_intro_step);
        if (!loaded.IsValid())
        {
            return false;
        }

        *this = loaded;
        return true;
    }

    bool AbstractActionPhaseState::IsValid() const noexcept
    {
        constexpr double kMaximumTimerSeconds = 3'600.0;
        constexpr double kMaximumWorldCoordinate = 1'000'000.0;

        const bool phase_valid =
            phase == ActionPhaseState::Intro || phase == ActionPhaseState::Active;
        const bool intro_step_valid =
            intro_step == ActionIntroStep::Standby ||
            intro_step == ActionIntroStep::ReadyBlink ||
            intro_step == ActionIntroStep::WarpIn ||
            intro_step == ActionIntroStep::Done;
        const bool timer_valid =
            std::isfinite(intro_timer) &&
            intro_timer >= 0.0 &&
            intro_timer <= kMaximumTimerSeconds;
        const bool player_position_valid =
            std::isfinite(player_previous_position.x) &&
            std::isfinite(player_previous_position.y) &&
            std::abs(player_previous_position.x) <= kMaximumWorldCoordinate &&
            std::abs(player_previous_position.y) <= kMaximumWorldCoordinate;
        const bool phase_consistent =
            phase != ActionPhaseState::Active || intro_step == ActionIntroStep::Done;

        return
            phase_valid &&
            intro_step_valid &&
            timer_valid &&
            player_position_valid &&
            phase_consistent &&
            ready_ui.IsValid() &&
            scroll.IsValid();
    }

    AbstractActionPhase::AbstractActionPhase(std::unique_ptr<StageRuntimeContext> ctx, IStageScript* script, IPhaseHost& host) noexcept
        : _ctx(std::move(ctx)), _script(script), _host(&host)
    {
    }

    AbstractActionPhase::~AbstractActionPhase()
    {
        if (_ctx && _script && _entered)
        {
            _script->OnExit(_ctx->area_key, *_ctx);
        }
    }

    bool AbstractActionPhase::CanCaptureState() const noexcept
    {
        // The miss sequence is a short, non-interactive window between the
        // player vanishing and the stage restarting -- nothing worth resuming
        // into, so it is simply not capturable (see ActionPhaseState::Miss).
        return _state != ActionPhaseState::Miss &&
            _ctx != nullptr && _ctx->scroll != nullptr &&
            _ctx->entity_mgr != nullptr && _ctx->asset_provider != nullptr &&
            _ctx->entity_mgr->CanCaptureState();
    }

    bool AbstractActionPhase::CaptureState(AbstractActionPhaseState& state) const noexcept
    {
        if (!CanCaptureState())
        {
            return false;
        }

        state = AbstractActionPhaseState{
            .phase = _state,
            .intro_step = _intro.step,
            .intro_timer = _intro.timer,
            .entered = _entered,
            .operate = _operate,
            .player_previous_position = _player_prev_pos,
            .ready_ui = _ready_ui.CaptureState(),
            .scroll = _ctx->scroll->CaptureState(),
            .enemy_attack_pattern_counter = _enemy_attack_pattern_counter,
        };
        return state.IsValid();
    }

    bool AbstractActionPhase::CaptureEntityState(
        world::entity::EntityManagerState& state) const
    {
        return CanCaptureState() && _ctx->entity_mgr->CaptureState(state);
    }

    bool AbstractActionPhase::RestoreScrollState(
        const AbstractActionPhaseState& state) noexcept
    {
        return state.IsValid() && _ctx != nullptr && _ctx->scroll != nullptr &&
            _ctx->scroll->RestoreState(state.scroll);
    }

    bool AbstractActionPhase::RestoreEntityState(
        const world::entity::EntityManagerState& state,
        const AbstractActionPhaseState& phase_state)
    {
        if (!state.IsValid() || !phase_state.IsValid() || !_ctx ||
            !_ctx->entity_mgr || !_ctx->asset_provider || !_ctx->scroll ||
            !_ctx->page_grid || !_ctx->terrain_probe || !_ctx->ladder_service)
        {
            return false;
        }

        const world::entity::EntityStateFactory factory(*_ctx->asset_provider);
        if (!_ctx->entity_mgr->RestoreState(state, factory))
        {
            return false;
        }

        auto* player = _ctx->entity_mgr->FindFirst<world::entity::avatar::PlayerEntity>();
        if (player == nullptr)
        {
            return false;
        }

        player->SetTerrainProbe(_ctx->terrain_probe.get());
        player->SetLadderService(_ctx->ladder_service.get());
        player->SetScrollRuleProvider(_ctx->scroll->Rules());

        // EnemyEntity's ITerrainProbe* (needed for gravity) isn't part of its
        // saved state either -- same reasoning as the player's above.
        _ctx->entity_mgr->ForEachAlive<world::entity::enemy::EnemyEntity>(
            [this](world::entity::enemy::EnemyEntity& enemy)
            {
                enemy.SetTerrainProbe(_ctx->terrain_probe.get());
            });
        // ProjectileEntity's ITerrainProbe* (needed for wall/floor despawn) isn't
        // part of its saved state either -- same reasoning as the enemy's above.
        _ctx->entity_mgr->ForEachAlive<world::entity::effects::ProjectileEntity>(
            [this](world::entity::effects::ProjectileEntity& projectile)
            {
                projectile.SetTerrainProbe(_ctx->terrain_probe.get());
            });
        if (phase_state.phase == ActionPhaseState::Active)
        {
            player->SetInput(_ctx->input);
        }
        if (const auto page = _ctx->page_grid->ResolvePageIndexFromWorldPos(player->pos); page)
        {
            _ctx->terrain_probe->SetCurrentPage(*page);
        }
        else
        {
            return false;
        }
        return true;
    }

    bool AbstractActionPhase::RestoreRuntimeState(
        const AbstractActionPhaseState& state) noexcept
    {
        if (!state.IsValid() || !_ctx || !_ctx->scroll ||
            !_ready_ui.RestoreState(state.ready_ui))
        {
            return false;
        }

        _state = state.phase;
        _intro.step = state.intro_step;
        _intro.timer = state.intro_timer;
        _entered = state.entered;
        _operate = state.operate;
        _player_prev_pos = state.player_previous_position;
        _enemy_attack_pattern_counter = state.enemy_attack_pattern_counter;

        // Loading mid-miss drops straight back into the saved (never-Miss)
        // state, so undo the miss sequence's own leftovers too.
        _miss_frames = 0;
        _retry_requested = false;
        setBgmMissVoicesMuted_(false);

        // Audio and charge particles are presentation state. Rebuild them from
        // the player's next frame output instead of serializing channel state.
        _charge_sound_playing = false;
        _charge_phase = world::entity::avatar::ChargePhase::Idle;
        // Sync to the just-restored scroll state (RestoreScrollState() already ran)
        // so the next tick doesn't see a spurious lock edge and re-clear effects /
        // re-pause SE that were never touched this session.
        _scroll_was_locked = _ctx->scroll->IsScrollLocked();
        return true;
    }

    bool AbstractActionPhase::RestoreState(const AbstractActionPhaseState& state) noexcept
    {
        return RestoreScrollState(state) && RestoreRuntimeState(state);
    }

    void AbstractActionPhase::RestoreChargePresentationState(
        const systems::audio::SeTransportState& se_state) noexcept
    {
        if (!_ctx || !_ctx->entity_mgr)
        {
            _charge_sound_playing = false;
            _charge_phase = world::entity::avatar::ChargePhase::Idle;
            return;
        }

        const auto* player =
            _ctx->entity_mgr->FindFirst<world::entity::avatar::PlayerEntity>();
        if (player == nullptr)
        {
            _charge_sound_playing = false;
            _charge_phase = world::entity::avatar::ChargePhase::Idle;
            return;
        }

        const auto& charge = player->ChargeState();
        const bool charge_sound_restored = std::any_of(
            se_state.continuous_instances.begin(),
            se_state.continuous_instances.end(),
            [](const systems::audio::ContinuousSeTransportState& instance)
            {
                return instance.name == kChargeSeName;
            });
        _charge_sound_playing = charge_sound_restored;
        _charge_phase = charge.phase;
    }

    void AbstractActionPhase::Initialize(const resources::parameters::Parameters& params)
    {
        // A restart after a miss builds a fresh phase while the previous one's
        // voice mutes are still in effect (see beginMiss_()).
        setBgmMissVoicesMuted_(false);

        if (params.Get<std::wstring>(L"bgm_key"))
        {
            _bgm_key = *params.Get<std::wstring>(L"bgm_key");
            auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
            audio.PlayBgm(_bgm_key);
        }

        if (!_ctx)
        {
            return;
        }

        if (_script && !_entered)
        {
            _script->OnEnter(_ctx->area_key, *_ctx);
            _entered = true;
        }

        _ready_ui.Begin(3.0);   // 3 seconds duration for "READY" blink
    }

    PhaseResult AbstractActionPhase::Update()
    {
        if (!_ctx)
        {
            return PhaseResult::None();
        }

        /* NOTE: 
         * Update tile animations for the background tiles.
         * Rotating the animation within DrawPage() causes the animation's tile counter to advance too rapidly,
         * so ensure it is executed only once in the higher-level update logic.
         */
        auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
        resource.GetBGTileManager().UpdateTileAnimations();

        if (_state == ActionPhaseState::Intro)
        {
            updateIntro_();
            return PhaseResult::None();
        }

        if (_state == ActionPhaseState::Miss)
        {
            updateMiss_();
            return PhaseResult::None();
        }

        updateActive_();

        const bool verified = false; // TODO: replace with real trigger
        if (verified && _host != nullptr)
        {
            PhaseFadePlan next(
                5,   /* preBlackHold */
                20,  /* fadeInFrames */
                0,   /* preFadeOutHold */
                20,  /* fadeOutFrames */
                0,   /* postBlackHold */
                FadeLayerMask::All /* layers */
            );
            resources::parameters::Parameters p;
            _host->RequestTransition(L"TopMenu", next, p);

            // Return None because transition will be handled by Scene via host.
            return PhaseResult::None();
        }

        return PhaseResult::None();
    }

    void AbstractActionPhase::RenderWorld()
    {
        if (!_ctx)
        {
            return;
        }

        _ctx->scroll->Render();

        if (_intro.step == ActionIntroStep::ReadyBlink && !_ready_ui.IsFinished())
        {
            _ready_ui.Render();
        }

        systems::view::RenderContext ctx{
            .view = &_ctx->scroll->GetView(),
            .layer = systems::view::Layer::Actors,
        };
        _ctx->entity_mgr->RenderLayer(ctx, systems::view::Layer::Actors);
        _ctx->entity_mgr->RenderLayer(ctx, systems::view::Layer::Effects);
    }

    void AbstractActionPhase::RenderOverlay()
    {
        if (!_ctx)
        {
            return;
        }

        using namespace utils;

        wchar_t buf[128]{};
        const auto& hud = config::ConfigUIManager::GetCurrentHudConfig();
        int dispY = 8;

        if (hud.showPlayerPosition)
        {
            core::overlay::DebugHud::GetInstance().SetPlayerPositionContext(
                { _page_index_debug, _player_pos_x_debug, _player_pos_y_debug, _player_hp_debug, _player_max_hp_debug }
            );
        }

        _ctx->scroll->DebugHudRender(hud.showScrollLine);
    }

    void AbstractActionPhase::SetEnableOperatePhase(bool enable)
    {
        _operate = enable;

        if (!_operate)
            return;

        _intro.step = ActionIntroStep::ReadyBlink;
    }

    void AbstractActionPhase::updateIntro_()
    {
        using namespace world::entity;
        const double dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();
        _intro.timer += dt;

        // Enemies animate/patrol/fall through the READY/warp-in sequence too --
        // the player itself is intentionally NOT run through its regular
        // Update() here (it has its own intro-specific animation path below),
        // and collision is intentionally NOT resolved here either, so nothing
        // can actually touch the player while it isn't controllable yet. This
        // is a non-issue for real level design anyway: enemies belong to the
        // page a stage's starting page scrolls into, not the start page itself.
        _ctx->entity_mgr->ForEachAlive<enemy::EnemyEntity>(
            [view = &_ctx->scroll->GetView(), dt](enemy::EnemyEntity& e) { e.Update(view, dt); });

        auto* player = _ctx->entity_mgr->FindFirst<avatar::PlayerEntity>();

        switch (_intro.step)
        {
            case ActionIntroStep::ReadyBlink:
            {
                _ready_ui.Update(dt);
                if (_ready_ui.IsFinished())
                {
                    _intro.step = ActionIntroStep::WarpIn;
                    _intro.timer = 0.0;
                    player->BeginIntroDrop();
                }
                break;

            case ActionIntroStep::WarpIn:
                player->UpdateIntroAnimation(dt);
                if (player->IsIntroFinished())
                {
                    _intro.step = ActionIntroStep::Done;
                }
                break;

            case ActionIntroStep::Done:
                player->SetInput(_ctx->input);
                _state = ActionPhaseState::Active;
                break;
            }
        }

        if (player != nullptr)
        {
            consumePlayerOutput_(*player);
        }
    }

    void AbstractActionPhase::consumePlayerOutput_(world::entity::avatar::PlayerEntity& player)
    {
        auto output = player.TakeFrameOutput();
        auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
        auto& audio = resource.GetAudioManager();

        using EventType = world::entity::avatar::PlayerEventType;

        // Release the looping charge SE before a shot SE reuses the same SE channel.
        updateChargePresentation_(player, output.charge);

        for (const auto& event : output.events)
        {
            switch (event.type)
            {
            case EventType::EnteredWater:
                audio.PlaySe(L"splash");
                break;

            case EventType::FiredRockBuster:
                audio.PlaySe(L"rock_buster_bang");
                break;

            case EventType::FiredMaxChargeShot:
                audio.PlaySe(L"charge_shot_bang");
                break;

            case EventType::Landed:
                audio.PlaySe(L"landing_thump");
                break;

            case EventType::IntroLanded:
                audio.PlaySe(L"onstage_thump");
                break;

            case EventType::SlidingStarted:
            case EventType::DashStarted:
            {
                const auto sprite_id = _ctx->asset_provider->SlidingDustEffectSprite();
                if (sprite_id == static_cast<rendering::sprite::SpriteManager::Id>(-1))
                {
                    break;
                }

                const double direction = static_cast<double>(event.facing);
                const double sprite_offset_x = event.type == EventType::DashStarted
                    ? -2.0
                    : 2.0;
                const auto command = world::entity::common::SpawnSlidingDustEffectCommand{
                    .spawnPos = event.position + foundation::math::Vec2{
                        sprite_offset_x * direction,
                        0.0
                    },
                    .spriteId = sprite_id,
                    .baseTexture = event.facing == world::entity::avatar::AvatarDirection::Right
                        ? 0
                        : 4
                };
                _ctx->entity_mgr->Spawn<world::entity::effects::SlidingDustEffectEntity>(command);
                break;
            }
            }
        }

        if (output.projectile.has_value())
        {
            auto& projectile = _ctx->entity_mgr->Spawn<world::entity::effects::ProjectileEntity>(*output.projectile);
            projectile.SetTerrainProbe(_ctx->terrain_probe.get());
        }

        if (output.splashEffect.has_value())
        {
            _ctx->entity_mgr->Spawn<world::entity::effects::SplashEffectEntity>(*output.splashEffect);
        }

    }

    void AbstractActionPhase::updateActive_()
    {
        using namespace world::entity;

        // Script tick
        if (_script)
        {
            _script->OnUpdate(_ctx->area_key, *_ctx);
        }

        using namespace foundation::math;
        Vec2 delta{ 0, 0 };

        const bool lock = _ctx->scroll->IsScrollLocked();
        handleScrollLockTransition_(lock);

        /* Entity Updates */
        auto* player = _ctx->entity_mgr->FindFirst<avatar::PlayerEntity>();
        if (player != nullptr)
        {
            const Vec2 prev_pos = player->pos;

            if (const auto p = _ctx->page_grid->ResolvePageIndexFromWorldPos(player->pos); p)
            {
                _ctx->terrain_probe->SetCurrentPage(*p);

                const auto measure = _ctx->scroll->CurrentPageBoundsWorld();
                player->SetViewBounds(measure.fromBounds);
                player->SetPageOriginPx(measure.pageOriginPx);
                player->SetScrollContext(_ctx->scroll->Rules(), _ctx->scroll->PageIndex());
                // NOTE: Fixed page scroll is only active when the player is on the current page.
                player->SetFixedPageScrollAvailable(p == _ctx->scroll->PageIndex());
            }

            const double dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();
            if (!lock)
            {
                player->SetInput(_ctx->input);

                // CountAlive<ProjectileEntity>() would count every projectile
                // regardless of owner -- since ProjectileEntity is now shared
                // with enemy shots (see ProjectileEntity::Layer()/CollisionLayer::
                // ProjectileEnemy), that let an enemy's own volley (e.g. Met's
                // 3-way shot) eat the player's on-screen shot budget and block
                // their Rock Buster. Only the player's own shots should count
                // against it.
                std::size_t player_projectile_count = 0;
                _ctx->entity_mgr->ForEachAlive<effects::ProjectileEntity>(
                    [&player_projectile_count](const effects::ProjectileEntity& projectile)
                    {
                        if (projectile.Layer() == systems::physics::CollisionLayer::ProjectilePlayer)
                        {
                            ++player_projectile_count;
                        }
                    });

                auto entity_ctx = avatar::ExPlayerContextForEntity{
                    .canSpawnProjectile = (player_projectile_count < 3),   // TODO: make configurable (attack limit for player)
                };

                player->SetEntityContext(entity_ctx);

                feedEnemies_(player->pos);

                _ctx->entity_mgr->UpdateAll(&_ctx->scroll->GetView(), dt);
                consumePlayerOutput_(*player);
                spawnEnemyProjectiles_();
                advanceSharedAttackCounter_();

                resolveEntityCollisions_();

                // Vitality ran out on this pass: the miss starts this very
                // tick, before the (now moot) knockback reaction ever shows.
                if (player->IsDead())
                {
                    beginMiss_(*player, MissCause::OutOfVitality);
                    return;
                }

                delta = player->pos - prev_pos;
            }
            else
            {
                delta = Vec2{ 0, 0 };
                if (!_ctx->scroll->IsFreezeFrames())
                {
                    player->TickAnimation(dt);
                }
            }
        }

        /* BG Updates */
        if (player != nullptr)
        {
            if (const auto req = player->ConsumeScrollRequest(); req)
            {
                _ctx->scroll->RequestFixedScroll(*req);
            }
        }

        _ctx->scroll->SetTargetPos(player ? player->pos : Vec2::Zero());
        const auto fx = _ctx->scroll->Update(delta);

        // Apply carry movement while scrolling
        if (player && fx.fixedActive)
        {
            player->pos += fx.playerDelta;
        }

        if (player != nullptr && hasFallenOutOfStage_(*player))
        {
            beginMiss_(*player, MissCause::FellIntoPit);
            return;
        }

        _page_index_debug = static_cast<int>(_ctx->scroll->PageIndex());
        _player_pos_x_debug = player ? _ctx->page_grid->ToLocalPos(player->pos.x, config::SystemConfig::kScreenWidth) : 0;
        _player_pos_y_debug = player ? _ctx->page_grid->ToLocalPos(player->pos.y, config::SystemConfig::kScreenHeight) : 0;
        _player_hp_debug = player ? player->CurrentHP() : 0;
        _player_max_hp_debug = player ? player->MaxHP() : 0;

        _player_prev_pos = player ? player->pos : Vec2::Zero();

        if (_ctx->input->JustPressed(JPBTN::BACK))
        {
            auto* audio = &runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
            audio->OutputBGMMasterVolume();
        }
    }

    void AbstractActionPhase::feedEnemies_(const Vec2& player_pos)
    {
        // Feeds the player position and the shared attack pattern counter to
        // every enemy, before their own Update() runs (see EnemyEntity::
        // SetPlayerPosition()/SetSharedAttackCounter()).
        _ctx->entity_mgr->ForEachAlive<world::entity::enemy::EnemyEntity>(
            [&player_pos, counter = _enemy_attack_pattern_counter](world::entity::enemy::EnemyEntity& enemy_entity)
            {
                enemy_entity.SetPlayerPosition(player_pos);
                enemy_entity.SetSharedAttackCounter(counter);
            });
    }

    void AbstractActionPhase::resolveEntityCollisions_()
    {
        // Entity-vs-entity hit detection (projectiles vs enemies/traps, player vs
        // items/traps, etc.). Runs after positions are finalized for this tick.
        std::vector<systems::physics::ICollider*> colliders;
        _ctx->entity_mgr->CollectColliders(colliders);
        const auto damageable_hp_before = captureDamageableHp_(colliders);
        _ctx->collision.ResolveEntities(colliders);
        spawnDestructionEffectsForTheDead_(colliders);
        spawnHitEffectsForTheSurvivors_(damageable_hp_before);
        spawnDeflectEffectsForTheBounced_(colliders);
    }

    void AbstractActionPhase::updateMiss_()
    {
        // The rest of the world carries on as if the player were still
        // standing where it was lost: enemies keep facing/attacking that
        // spot, and shots already in flight (the player's own included) keep
        // flying and hitting. The player itself is dead, so it is neither
        // updated nor collected as a collider -- nothing can hurt it now.
        // Only the camera stays put (no target to follow).
        const double dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();
        feedEnemies_(_miss_origin);
        _ctx->entity_mgr->UpdateAll(&_ctx->scroll->GetView(), dt);
        spawnEnemyProjectiles_();
        advanceSharedAttackCounter_();
        resolveEntityCollisions_();

        ++_miss_frames;
        if (_retry_requested || _miss_frames < _miss_fade_out_delay_frames || _host == nullptr)
        {
            return;
        }

        // Same timing as the scene's own first entry into the stage, so a
        // restart reads the same as starting it fresh (READY included -- the
        // host builds a brand-new phase for it).
        _retry_requested = true;
        const PhaseFadePlan retry(
            20,  /* preBlackHold */
            20,  /* fadeInFrames */
            0,   /* preFadeOutHold */
            12,  /* fadeOutFrames */
            20,  /* postBlackHold */
            FadeLayerMask::All /* layers */
        );
        const resources::parameters::Parameters none;
        _host->RequestTransition(L"Retry", retry, none);
    }

    bool AbstractActionPhase::hasFallenOutOfStage_(const world::entity::avatar::PlayerEntity& player) const
    {
        // A room below takes over through a fixed page scroll, requested as
        // soon as the player's feet cross the page's bottom edge -- well before
        // the whole hit box can clear the view. A free-scrolling page keeps the
        // camera on the player instead. So still sinking out of the view
        // without either happening means there is nothing down there.
        if (_ctx->scroll->IsScrollLocked())
        {
            return false;
        }

        const auto& view = _ctx->scroll->GetView();
        const double view_bottom = view.viewWorldY + static_cast<double>(view.viewH);
        return player.Bounds().y >= view_bottom;
    }

    void AbstractActionPhase::beginMiss_(world::entity::avatar::PlayerEntity& player, MissCause cause)
    {
        using world::entity::EntityTypeId;

        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
        const Vec2 origin = player.pos;
        _miss_origin = origin;

        // The charge loop would otherwise keep running (and its particles keep
        // hovering around nobody) through the whole sequence.
        if (_charge_sound_playing)
        {
            audio.StopSe(kChargeSeName);
            _charge_sound_playing = false;
        }
        _charge_phase = world::entity::avatar::ChargePhase::Idle;
        static constexpr std::array kChargeEffectTypes{ EntityTypeId::ChargeEffect };
        _ctx->entity_mgr->KillAllOfTypes(kChargeEffectTypes);

        player.Kill();

        const auto sprite_id = _ctx->asset_provider->MissBubbleEffectSprite();
        if (sprite_id != static_cast<rendering::sprite::SpriteManager::Id>(-1))
        {
            for (const double speed : { kMissOuterRingSpeed, kMissInnerRingSpeed })
            {
                for (const auto& direction : kMissBubbleDirections)
                {
                    _ctx->entity_mgr->Spawn<world::entity::effects::MissBubbleEffectEntity>(
                        world::entity::common::SpawnMissBubbleEffectCommand{
                            .spawnPos = origin,
                            .velocity = direction * speed,
                            .spriteId = sprite_id,
                        });
                }
            }
        }

        audio.PlaySe(L"terrible_scatter");
        setBgmMissVoicesMuted_(true);

        _state = ActionPhaseState::Miss;
        _miss_frames = 0;
        _miss_fade_out_delay_frames = MissFadeOutDelayFrames(cause);
        _retry_requested = false;
    }

    void AbstractActionPhase::setBgmMissVoicesMuted_(bool muted)
    {
        using systems::audio::ApuVoice;
        using systems::audio::SoundChip;

        // Only the pulses drop out, as if the miss SE had taken them over for
        // good -- a 2A03 channel-budget look. Triangle, noise and DPCM carry on.
        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
        for (const ApuVoice voice : { ApuVoice::Pulse1, ApuVoice::Pulse2 })
        {
            audio.MuteChannel(SoundChip::APU, static_cast<int>(systems::audio::ToIndex(voice)), muted);
        }
    }

    void AbstractActionPhase::updateChargePresentation_(
        const world::entity::avatar::PlayerEntity& player,
        const world::entity::avatar::ChargeStatus& charge)
    {
        using world::entity::avatar::ChargePhase;
        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
        const bool charge_effect_active =
            charge.phase == ChargePhase::Level1 || charge.phase == ChargePhase::Level2;
        if (!charge_effect_active)
        {
            if (_charge_sound_playing)
            {
                audio.StopSe(kChargeSeName);
                _charge_sound_playing = false;
            }
            _charge_phase = charge.phase;
            return;
        }

        if (!_charge_sound_playing)
        {
            audio.PlaySe(kChargeSeName);
            _charge_sound_playing = true;
        }

        const int spawn_interval_ticks = charge.phase == ChargePhase::Level2 ? 4 : 8;
        const bool spawn_now = charge.phase != _charge_phase ||
            charge.phaseFrames % static_cast<std::uint32_t>(spawn_interval_ticks) == 0;
        _charge_phase = charge.phase;
        if (!spawn_now) return;

        const auto sprite_id = _ctx->asset_provider->ChargeEffectSprite();
        if (sprite_id == static_cast<rendering::sprite::SpriteManager::Id>(-1)) return;

        const auto pattern_index =
            (charge.phaseFrames / static_cast<std::uint32_t>(spawn_interval_ticks)) %
            kChargeParticlePattern.size();
        const auto& particle = kChargeParticlePattern[pattern_index];

        _ctx->entity_mgr->Spawn<world::entity::effects::ChargeEffectEntity>(
            world::entity::common::SpawnChargeEffectCommand{
                .spawnPos = player.pos + Vec2{ static_cast<double>(particle.offset_x), 12.0 },
                .spriteId = sprite_id,
                .baseTexture = particle.base_texture
            });
    }

    void AbstractActionPhase::spawnDestructionEffectsForTheDead_(
        const std::vector<systems::physics::ICollider*>& colliders)
    {
        using systems::combat::IDamageable;

        const auto sprite_id = _ctx->asset_provider->SmallExplosionEffectSprite();
        if (sprite_id == static_cast<rendering::sprite::SpriteManager::Id>(-1))
        {
            return;
        }

        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();

        for (auto* collider : colliders)
        {
            // `colliders` was snapshotted alive just before ResolveEntities() ran
            // above, so a now-dead entry died on this exact pass -- fire its
            // destruction effect exactly once, right here.
            if (collider == nullptr || collider->OwnerEntity().IsAlive())
            {
                continue;
            }

            // Only entities with an HP pool (combat::IDamageable) get a destruction
            // effect -- e.g. a projectile despawning on its own isn't "destroyed".
            if (dynamic_cast<IDamageable*>(collider) == nullptr)
            {
                continue;
            }

            const auto center = collider->Bounds().center();
            _ctx->entity_mgr->Spawn<world::entity::effects::SmallExplosionEffectEntity>(
                world::entity::common::SpawnSmallExplosionEffectCommand{
                    .spawnPos = Vec2{ center.x, center.y },
                    .spriteId = sprite_id,
                    .baseTexture = 0,
                });
            audio.PlaySe(L"enemy_small_explosion");
        }
    }

    std::vector<AbstractActionPhase::DamageableHpSnapshot> AbstractActionPhase::captureDamageableHp_(
        const std::vector<systems::physics::ICollider*>& colliders) const
    {
        using systems::combat::IDamageable;

        std::vector<DamageableHpSnapshot> snapshot;
        snapshot.reserve(colliders.size());
        for (auto* collider : colliders)
        {
            if (collider == nullptr)
            {
                continue;
            }

            if (auto* damageable = dynamic_cast<IDamageable*>(collider))
            {
                snapshot.push_back({ damageable, damageable->CurrentHP() });
            }
        }
        return snapshot;
    }

    void AbstractActionPhase::spawnHitEffectsForTheSurvivors_(
        const std::vector<DamageableHpSnapshot>& before)
    {
        // A kill is destruction's job (spawnDestructionEffectsForTheDead_);
        // this is only for a hit that didn't finish the target off. Split by
        // whether the target was the player -- "be_damaged" is the player's
        // own hit-reaction SE, distinct from "hit_attack" (an enemy taking a
        // non-lethal hit from the player's own weapon).
        bool any_enemy_hit = false;
        bool player_hit = false;
        for (const auto& snap : before)
        {
            if (snap.damageable->IsDead() || snap.damageable->CurrentHP() >= snap.hp_before)
            {
                continue;
            }
            if (dynamic_cast<world::entity::avatar::PlayerEntity*>(snap.damageable) != nullptr)
            {
                player_hit = true;
            }
            else
            {
                any_enemy_hit = true;
            }
        }

        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
        // One shot per frame regardless of how many things got hit -- a single
        // SE channel pair, no point stacking retriggers.
        if (any_enemy_hit) { audio.PlaySe(L"hit_attack"); }
        if (player_hit) { audio.PlaySe(L"be_damaged"); }
    }

    void AbstractActionPhase::spawnDeflectEffectsForTheBounced_(
        const std::vector<systems::physics::ICollider*>& colliders)
    {
        bool any_deflected = false;
        for (auto* collider : colliders)
        {
            auto* projectile = dynamic_cast<world::entity::effects::ProjectileEntity*>(collider);
            if (projectile != nullptr && projectile->ConsumeDeflected())
            {
                any_deflected = true;
            }
        }

        if (any_deflected)
        {
            runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager().PlaySe(L"defend_shot");
        }
    }

    void AbstractActionPhase::spawnEnemyProjectiles_()
    {
        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
        _ctx->entity_mgr->ForEachAlive<world::entity::enemy::EnemyEntity>(
            [this, &audio](world::entity::enemy::EnemyEntity& enemy)
            {
                auto spawns = enemy.ConsumePendingProjectileSpawns();
                if (spawns.empty())
                {
                    return;
                }

                // One shot per firing enemy this frame, not one per bullet --
                // e.g. Met's 3-way volley on waking up is a single "it fired"
                // cue, not three overlapping copies of the same SE.
                audio.PlaySe(L"enemy_fire");
                for (auto& cmd : spawns)
                {
                    auto& projectile = _ctx->entity_mgr->Spawn<world::entity::effects::ProjectileEntity>(cmd);
                    projectile.SetTerrainProbe(_ctx->terrain_probe.get());
                }
            });
    }

    void AbstractActionPhase::advanceSharedAttackCounter_()
    {
        _ctx->entity_mgr->ForEachAlive<world::entity::enemy::EnemyEntity>(
            [this](world::entity::enemy::EnemyEntity& enemy)
            {
                if (enemy.ConsumeAttackCounterIncrementRequest())
                {
                    ++_enemy_attack_pattern_counter;
                }
            });
    }

    void AbstractActionPhase::handleScrollLockTransition_(bool locked_now)
    {
        if (locked_now == _scroll_was_locked)
        {
            return;
        }
        _scroll_was_locked = locked_now;

        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();

        if (locked_now)
        {
            // Page-scroll transition just started: drop every transient effect --
            // including a Rock Buster shot that wandered off-screen -- but leave
            // the player and persistent level objects (e.g. BreakableBlock) alone.
            static constexpr std::array kEffectTypes{
                world::entity::EntityTypeId::Projectile,
                world::entity::EntityTypeId::ChargeEffect,
                world::entity::EntityTypeId::SlidingDustEffect,
                world::entity::EntityTypeId::SplashEffect,
                world::entity::EntityTypeId::SmallExplosionEffect,
            };
            _ctx->entity_mgr->KillAllOfTypes(kEffectTypes);

            // Freeze SE (not BGM) so a continuous one -- the charge loop, notably --
            // doesn't keep advancing while gameplay (and the player's actual charge
            // frames) are frozen for the duration of the scroll.
            audio.PauseSe();
        }
        else
        {
            audio.ResumeSe();
        }
    }
}
