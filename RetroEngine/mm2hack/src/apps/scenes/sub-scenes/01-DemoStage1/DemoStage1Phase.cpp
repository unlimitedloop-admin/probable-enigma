#include "pch.h"

#include "DemoStage1Phase.h"

#include <cstdio>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/resources/bg/MapPageCache.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "config/ConfigUIManager.h"
#include "DemoStage1.h"
#include "utils/decimal_decoder.h"
#include "utils/string_converter.h"

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
            _scroll->SetPageIndex(0);
            using Camera = systems::scrolling::atomic::Camera;
            _scroll->ObjectPos() = { Camera::kCenterX, Camera::kCenterY };

            auto* bgMgr = &resource.GetBGTileManager();
            _mapProvider = std::make_unique<BGTileMapProvider>(bgMgr, pageSource);
            _terrainProbe = std::make_unique<TileQueryService>(*_mapProvider, *_graph, *_pageGrid, kTilePx);
            _ladderService = std::make_unique<LadderService>(*_terrainProbe);

            // TODO: Need to provide a vector member for entity. (or EntityManager?)
            _player = std::make_unique<PlayerEntity>(owner.GetSpriteId());
            _player->SetTerrainProbe(_terrainProbe.get());
            _player->SetLadderService(_ladderService.get());
            _player->pos = _initialize_pos;
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
                    _player->SetViewBounds(_scroll->CurrentPageBoundsWorld());
                }

                const double dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();
                if (!lock)
                {
                    _player->SetInput(owner.Input());
                    _player->Update(dt);

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


            // Test version...
            //Vec2 delta{ 0,0 };
            //Vec2 prev = _player ? _player->pos : Vec2::Zero();
            //auto dt = runtime::GameContext::GetInstance().Time().DeltaSeconds();

            //// request transfer は今まで通り
            //if (_player)
            //{
            //    if (auto req = _player->ConsumeScrollRequest(); req)
            //    {
            //        _scroll->RequestFixedScroll(*req);
            //    }
            //}

            //// --- Scroll first (only when locked) ---
            //const bool locked = _scroll->IsScrollLocked(); // or your fx.fixedActive
            //if (locked)
            //{
            //    _scroll->SetTargetPos(_player ? _player->pos : Vec2::Zero());
            //    const auto fx = _scroll->Update(Vec2{ 0,0 });

            //    if (_player)
            //    {
            //        _player->pos += fx.playerDelta;
            //        _player->TickAnimation(dt);
            //        delta = _player->pos - prev;
            //    }
            //    return;
            //}

            //// --- On controlling action scene path (your current order) ---
            //if (_player)
            //{
            //    _player->SetInput(owner.Input());
            //    _player->Update(dt);
            //    delta = _player->pos - prev;
            //}
            //_scroll->SetTargetPos(_player ? _player->pos : Vec2::Zero());
            //_scroll->Update(delta);

            _page_index_debug = static_cast<int>(_scroll->PageIndex());
            _player_pos_x_debug = _player ? _player->pos.x : 0;
            _player_pos_y_debug = _player ? _player->pos.y : 0;

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
            int dispY = 8;
            ::swprintf(buf, 128, L"PageIndex = %d", _page_index_debug);
            ::DxLib::DrawString(8, dispY, buf, 0xFFFFFF00);
            dispY += 16;
            const std::wstring xstr = decode_floating_hex_number(_player_pos_x_debug);
            const std::wstring ystr = decode_floating_hex_number(_player_pos_y_debug);
            concat_to_wchar_buffer(buf, sizeof(buf) / sizeof(buf[0]), { L"Player Pos = (", xstr, L", ", ystr, L")" });
            ::DxLib::DrawString(8, dispY, buf, 0xFFFF0000);

            const auto& hud = config::ConfigUIManager::GetCurrentHudConfig();
            _scroll->DebugHudRender(hud.showScrollLine);
        }

        DemoStage1PhaseId MainPhase::Id() const noexcept
        {
            return DemoStage1PhaseId::Main;
        }
    }
}