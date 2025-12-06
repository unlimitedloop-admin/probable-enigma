#include "pch.h"

#include "DemoStage1Phase.h"

#include <cstdio>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/resources/bg/MapPageCache.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/systems/view/RenderContext.h"
#include "DemoStage1.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        using resources::bg::AddressScraper;
        using resources::bg::MapPageCache;

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
            _rules = std::make_unique<ScraperScrollRuleProvider>(pageSource);
            _renderer = std::make_unique<MapRenderer2D>(resource, owner.GetMapName(), owner.GetMapBinaryPath(), kTilePx);

            ScrollController::Params p;
            _scroll = std::make_unique<ScrollController>(*_rules, *_renderer, p);
            _scroll->SetPageIndex(0);
            _scroll->ObjectPos() = _initialize_pos;

            auto* bgMgr = &resource.GetBGTileManager();
            _mapProvider = std::make_unique<BGTileMapProvider>(bgMgr, pageSource);
            _terrainProbe = std::make_unique<TileQueryService>(*_mapProvider);

            // TODO: Need to provide a vector member for entity. (or EntityManager?)
            _player = std::make_unique<PlayerEntity>(owner.GetSpriteId());
            _player->SetTerrainProbe(_terrainProbe.get());
            _player->pos = _initialize_pos;
            _player->texture = 1;
            // for (auto& e : _enemies) { e->SetTerrainProbe(_terrainProbe.get()); }
        }

        void MainPhase::Update()
        {
            using namespace world::entity::avatar;

            if (!owner.Fader().InputEnabled()) return;

            // Simple object movement with arrow keys
            using namespace foundation::math;
            Vec2 delta{ 0, 0 };

            _scroll->Update(delta);
            _page_index_debug = static_cast<int>(_scroll->PageIndex());

            if (_player)
            {
                auto delta_time = runtime::GameContext::GetInstance().Time().DeltaSeconds();
                _player->SetInput(owner.Input());
                _player->Update(delta_time);
            }
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
            wchar_t buf[128]{};
            ::swprintf(buf, 128, L"PageIndex = %d", _page_index_debug);
            ::DxLib::DrawString(8, 8, buf, 0xFFFFFF00);
        }

        DemoStage1PhaseId MainPhase::Id() const noexcept
        {
            return DemoStage1PhaseId::Main;
        }
    }
}