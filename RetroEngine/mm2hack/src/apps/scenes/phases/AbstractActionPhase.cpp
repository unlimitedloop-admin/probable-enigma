#include "pch.h"

#include "AbstractActionPhase.h"

#include "apps/resources/parameters/Parameters.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "config/ConfigUIManager.h"
#include "core/overlay/DebugHud.h"
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
        (void)params;

        if (!_ctx)
        {
            return;
        }

        if (_script && !_entered)
        {
            _script->OnEnter(_ctx->area_key, *_ctx);
            _entered = true;
        }
    }

    PhaseResult AbstractActionPhase::Update()
    {
        if (!_ctx)
        {
            return PhaseResult::None();
        }

        // Script tick
        if (_script)
        {
            _script->OnUpdate(_ctx->area_key, *_ctx);
        }

        using namespace foundation::math;
        Vec2 delta{ 0, 0 };

        const bool lock = _ctx->scroll->IsScrollLocked();

        /* Entity Updates */
        if (_ctx->player)
        {
            const Vec2 prev_pos = _ctx->player->pos;

            if (const auto p = _ctx->page_grid->ResolvePageIndexFromWorldPos(_ctx->player->pos); p)
            {
                _ctx->terrain_probe->SetCurrentPage(*p);

                const auto measure = _ctx->scroll->CurrentPageBoundsWorld();
                _ctx->player->SetViewBounds(measure.fromBounds);
                _ctx->player->SetPageOriginPx(measure.pageOriginPx);
                _ctx->player->SetScrollContext(_ctx->scroll->Rules(), _ctx->scroll->PageIndex());
                // NOTE: Fixed page scroll is only active when the player is on the current page.
                _ctx->player->SetFixedPageScrollAvailable(p == _ctx->scroll->PageIndex());
            }

            const double dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();
            if (!lock)
            {
                _ctx->player->SetInput(_ctx->input);
                _ctx->player->Update(dt);

                delta = _ctx->player->pos - prev_pos;
            }
            else
            {
                // During fixed scroll: player is carried by scroll.
                delta = Vec2{ 0, 0 };
                if (!_ctx->scroll->IsFreezeFrames())
                {
                    _ctx->player->TickAnimation(dt);
                }
            }
        }

        /* BG Updates */
        if (_ctx->player)
        {
            if (const auto req = _ctx->player->ConsumeScrollRequest(); req)
            {
                _ctx->scroll->RequestFixedScroll(*req);
            }
        }

        _ctx->scroll->SetTargetPos(_ctx->player ? _ctx->player->pos : Vec2::Zero());
        const auto fx = _ctx->scroll->Update(delta);

        // Apply carry movement while scrolling
        if (_ctx->player && fx.fixedActive)
        {
            _ctx->player->pos += fx.playerDelta;
        }

        _page_index_debug = static_cast<int>(_ctx->scroll->PageIndex());
        _player_pos_x_debug = _ctx->player ? _ctx->page_grid->ToLocalPos(_ctx->player->pos.x, config::SystemConfig::kScreenWidth) : 0;
        _player_pos_y_debug = _ctx->player ? _ctx->page_grid->ToLocalPos(_ctx->player->pos.y, config::SystemConfig::kScreenHeight) : 0;

        _player_prev_pos = _ctx->player ? _ctx->player->pos : Vec2::Zero();

        // Example: if some condition verified -> request transition to menu
        const bool verified = false; // replace with real trigger
        if (verified && _host != nullptr)
        {
            PhaseFadePlan next(
                5,   // preBlackHold
                20,  // fadeInFrames
                0,   // preFadeOutHold
                20,  // fadeOutFrames
                0,   // postBlackHold
                FadeLayerMask::All // layers
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

        if (_ctx->player)
        {
            systems::view::RenderContext ctx{
                .view = &_ctx->scroll->GetView(),
                .layer = systems::view::Layer::Actors,
            };

            _ctx->player->Render(ctx);
        }
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
            core::overlay::DebugHud::GetInstance().SetPlayerPositionContext({
                _page_index_debug,
                _player_pos_x_debug,
                _player_pos_y_debug
                });
        }

        _ctx->scroll->DebugHudRender(hud.showScrollLine);
    }
}