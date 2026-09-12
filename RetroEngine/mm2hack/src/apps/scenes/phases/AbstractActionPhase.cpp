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
#include "apps/systems/audio/AudioManager.h"
#include "apps/systems/audio/SeTransportState.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "apps/world/entity/avatar/PlayerFrameOutput.h"
#include "apps/world/entity/common/SpawnChargeEffectCommand.h"
#include "apps/world/entity/common/SpawnSlidingDustEffectCommand.h"
#include "apps/world/entity/effects/ChargeEffectEntity.h"
#include "apps/world/entity/effects/ProjectileEntity.h"
#include "apps/world/entity/effects/SlidingDustEffectEntity.h"
#include "apps/world/entity/effects/SplashEffectEntity.h"
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
            scroll.Save(writer);
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
            !loaded.scroll.Load(reader))
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
        return _ctx != nullptr && _ctx->scroll != nullptr &&
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

        // Audio and charge particles are presentation state. Rebuild them from
        // the player's next frame output instead of serializing channel state.
        _charge_sound_playing = false;
        _charge_phase = world::entity::avatar::ChargePhase::Idle;
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
                { _page_index_debug, _player_pos_x_debug, _player_pos_y_debug }
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
            _ctx->entity_mgr->Spawn<world::entity::effects::ProjectileEntity>(*output.projectile);
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

                auto entity_ctx = avatar::ExPlayerContextForEntity{
                    .canSpawnProjectile = (_ctx->entity_mgr->CountAlive<effects::ProjectileEntity>() < 3),   // TODO: make configurable (attack limit for player)
                };

                player->SetEntityContext(entity_ctx);

                _ctx->entity_mgr->UpdateAll(&_ctx->scroll->GetView(), dt);
                consumePlayerOutput_(*player);

                // Entity-vs-entity hit detection (projectiles vs enemies/traps, player vs
                // items/traps, etc.). Runs after positions are finalized for this tick.
                std::vector<systems::physics::ICollider*> colliders;
                _ctx->entity_mgr->CollectColliders(colliders);
                _ctx->collision.ResolveEntities(colliders);

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

        _page_index_debug = static_cast<int>(_ctx->scroll->PageIndex());
        _player_pos_x_debug = player ? _ctx->page_grid->ToLocalPos(player->pos.x, config::SystemConfig::kScreenWidth) : 0;
        _player_pos_y_debug = player ? _ctx->page_grid->ToLocalPos(player->pos.y, config::SystemConfig::kScreenHeight) : 0;

        _player_prev_pos = player ? player->pos : Vec2::Zero();

        if (_ctx->input->JustPressed(JPBTN::BACK))
        {
            auto* audio = &runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
            audio->OutputBGMMasterVolume();
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
}
