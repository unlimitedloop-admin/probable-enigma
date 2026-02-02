#include "pch.h"

#include "DemoStage1Phase.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/resources/bg/MapPageCache.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/scrolling/atomic/Camera.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "config/ConfigUIManager.h"
#include "core/overlay/DebugHud.h"
#include "DemoStage1.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        using config::SystemConfig;

        using resources::bg::AddressScraper;
        using resources::bg::MapPageCache;
        using systems::physics::PageGridIndex;

        //============================================================================== 
        //
        //  MainPhase
        //
        //==============================================================================
        void MainPhase::Initialize()
        {
            auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
            auto& filepath = resource.GetBGRoomBank().FilePath();
            auto scraper = std::make_shared<AddressScraper>(resource.GetBGTileManager().ExtractMapBinary(filepath));
            auto pageSource = std::make_shared<MapPageCache>(scraper);

            constexpr double kPageW = SystemConfig::kScreenWidth;
            constexpr double kPageH = SystemConfig::kScreenHeight;
            _pageGrid = std::make_unique<PageGridIndex>(kPageW, kPageH);
            _graph = std::make_unique<RoomGraphAdapter>(*scraper);
            _pageGrid->Build(*_graph, /*start*/ 0);

            _rules = std::make_unique<ScraperScrollRuleProvider>(pageSource);
            _renderer = std::make_unique<MapRenderer2D>(resource, owner.GetMapName(), owner.GetMapBinaryPath(), kTilePx);

            ScrollController::Params p;
            _scroll = std::make_unique<ScrollController>(*_rules, *_renderer, p);
            _scroll->SetPageIndex(owner.GetCurrentRoomPageIndex());
            using Camera = systems::scrolling::atomic::Camera;
            _scroll->ObjectPos() = { Camera::kCenterX, Camera::kCenterY };

            auto* bgMgr = &resource.GetBGTileManager();
            _mapProvider = std::make_unique<BGTileMapProvider>(bgMgr, pageSource);
            _terrainProbe = std::make_unique<TileQueryService>(*_mapProvider, *_graph, *_pageGrid, kTilePx);
            _ladderService = std::make_unique<LadderService>(*_terrainProbe);

            // TODO: Need to provide a vector member for entity. (or EntityManager?)
            _player = std::make_unique<PlayerEntity>(owner.GetSpriteId(), owner.GetSpriteAttackId());
            _player->SetTerrainProbe(_terrainProbe.get());
            _player->SetLadderService(_ladderService.get());
            _player->SetScrollContext(_rules.get(), _scroll->PageIndex());
            if (const auto worldPos = _pageGrid->ToWorldPosOnPage(static_cast<int>(_scroll->PageIndex()), _initialize_pos))
            {
                _player->pos = *worldPos;
            }
            else
            {
                _player->pos = _initialize_pos;     // fallback if conversion fails.
            }
            _player->texture = 1;
            // for (auto& e : _enemies) { e->SetTerrainProbe(_terrainProbe.get()); }

            _player_prev_pos = _player->pos;
        }

        void MainPhase::Update()
        {
            using namespace foundation::math;
            Vec2 delta{ 0, 0 };

            const bool lock = _scroll->IsScrollLocked();

            /* Entity Updates */
            if (_player)
            {
                const Vec2 prev_pos = _player->pos;

                if (const auto p = _pageGrid->ResolvePageIndexFromWorldPos(_player->pos); p)
                {
                    _terrainProbe->SetCurrentPage(*p);

                    const auto measure = _scroll->CurrentPageBoundsWorld();
                    _player->SetViewBounds(measure.fromBounds);
                    _player->SetPageOriginPx(measure.pageOriginPx);
                    _player->SetScrollContext(_scroll->Rules(), _scroll->PageIndex());
                    // NOTE: Fixed page scroll is only active when the player is on the current page.
                    _player->SetFixedPageScrollAvailable(p == _scroll->PageIndex());
                }

                const double dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();
                if (!lock)
                {
                    _player->SetInput(owner.Input());
                    _player->Update(&_scroll->GetView(), dt);

                    delta = _player->pos - prev_pos;
                }
                else
                {
                    // During fixed scroll: player is carried by scroll.
                    delta = Vec2{ 0, 0 };
                    if (!_scroll->IsFreezeFrames())
                    {
                        _player->TickAnimation(dt);
                    }
                }
            }

            /* BG Updates */
            if (_player)
            {
                if (const auto req = _player->ConsumeScrollRequest(); req)
                {
                    _scroll->RequestFixedScroll(*req);
                }
            }

            _scroll->SetTargetPos(_player ? _player->pos : Vec2::Zero());
            const auto fx = _scroll->Update(delta);

            // Apply carry movement while scrolling
            if (_player && fx.fixedActive)
            {
                _player->pos += fx.playerDelta;
            }

            _page_index_debug = static_cast<int>(_scroll->PageIndex());
            _player_pos_x_debug = _player ? _pageGrid->ToLocalPos(_player->pos.x, config::SystemConfig::kScreenWidth) : 0;
            _player_pos_y_debug = _player ? _pageGrid->ToLocalPos(_player->pos.y, config::SystemConfig::kScreenHeight) : 0;

            _player_prev_pos = _player ? _player->pos : Vec2::Zero();
        }

        void MainPhase::RenderWorld()
        {
            _scroll->Render();

            if (_player)
            {
                systems::view::RenderContext ctx{
                    .view  = &_scroll->GetView(),
                    .layer = systems::view::Layer::Actors,
                };

                _player->Render(ctx);
            }
        }

        void MainPhase::RenderOverlay()
        {
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

            _scroll->DebugHudRender(hud.showScrollLine);
        }

        DemoStage1PhaseId MainPhase::Id() const noexcept
        {
            return DemoStage1PhaseId::Main;
        }
    }
}