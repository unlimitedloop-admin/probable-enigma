#include "pch.h"

#include "AbstractActionPhase.h"

#include "apps/resources/parameters/Parameters.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "apps/world/entity/effects/ProjectileEntity.h"
#include "apps/world/entity/EntityManager.h"
#include "config/ConfigUIManager.h"
#include "core/overlay/DebugHud.h"
#include "input/Jpbtn.h"
#include "IPhaseHost.h"
#include "IStageScript.h"
#include "PhaseResult.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::scenes::phases
{
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

        if (_intro.step == IntroStep::ReadyBlink && !_ready_ui.IsFinished())
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

        _intro.step = IntroStep::ReadyBlink;
    }

    void AbstractActionPhase::updateIntro_()
    {
        using namespace world::entity;
        const double dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();
        _intro.timer += dt;

        auto* player = _ctx->entity_mgr->FindFirst<avatar::PlayerEntity>();

        switch (_intro.step)
        {
            case IntroStep::ReadyBlink:
            {
                _ready_ui.Update(dt);
                if (_ready_ui.IsFinished())
                {
                    _intro.step = IntroStep::WarpIn;
                    _intro.timer = 0.0;
                    player->BeginIntroDrop();
                }
                break;

            case IntroStep::WarpIn:
                player->UpdateIntroAnimation(dt);
                if (player->IsIntroFinished())
                {
                    _intro.step = IntroStep::Done;
                }
                break;

            case IntroStep::Done:
                player->SetInput(_ctx->input);
                _state = ActionPhaseState::Active;
                break;
            }
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
                if (auto cmd = player->TakeSpawnProjectile(); cmd)
                {
                    _ctx->entity_mgr->Spawn<effects::ProjectileEntity>(*cmd);
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
}