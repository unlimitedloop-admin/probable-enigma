#include "pch.h"

#include "DemoStage1Phase.h"

#include <cstdio>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/resources/bg/MapPageCache.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "core/assembly/StateProvider.h"
#include "DemoStage1.h"
#include "input/Jpbtn.h"

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
            auto& resource = owner.ResourceManagerObj();
            auto& filepath = resource.GetBGRoomBank().FilePath();
            auto scraper = std::make_shared<AddressScraper>(
                resource.GetBGTileManager().ExtractMapBinary(filepath)
            );
            auto pageSource = std::make_shared<MapPageCache>(scraper);
            _rules = std::make_unique<ScraperScrollRuleProvider>(pageSource);
            _renderer = std::make_unique<MapRenderer2D>(resource, owner.GetMapName(), owner.GetMapBinaryPath(), kTilePx);

            ScrollController::Params p;
            _scroll = std::make_unique<ScrollController>(*_rules, *_renderer, p);
            _scroll->SetPageIndex(0);
            _scroll->ObjectPos() = { 128.0, 120.0 };

            auto* bgMgr = &resource.GetBGTileManager();
            _mapProvider = std::make_unique<BGTileMapProvider>(bgMgr, pageSource);
            _terrainProbe = std::make_unique<TileQueryService>(*_mapProvider);

            // TODO: Need to provide a vector member for entity. (or EntityManager?)
            // _player->SetTerrainProbe(_terrainProbe.get());
            // for (auto& e : _enemies) { e->SetTerrainProbe(_terrainProbe.get()); }
        }

        void MainPhase::Update()
        {
            if (!owner.Fader().InputEnabled()) return;

            // Simple object movement with arrow keys
            using namespace foundation::math;
            Vec2 delta{ 0, 0 };

            auto& input = owner.Input();
            if (input->IsPressed(JPBTN::UP))    delta.y -= 1;
            if (input->IsPressed(JPBTN::DOWN))  delta.y += 1;
            if (input->IsPressed(JPBTN::LEFT))  delta.x -= 1;
            if (input->IsPressed(JPBTN::RIGHT)) delta.x += 1;

            _scroll->Update(delta);
            _page_index_debug = static_cast<int>(_scroll->PageIndex());
        }

        void MainPhase::RenderWorld()
        {
            _scroll->Render();
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