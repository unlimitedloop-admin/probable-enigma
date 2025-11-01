#include "pch.h"

#include "DemoStage1Phase.h"

#include <cstdio>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "DemoStage1.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        //============================================================================== 
        //
        //  MainPhase
        //
        //==============================================================================
        void MainPhase::Initialize()
        {
            auto& resource = owner.ResourceManager();

            auto scraper = std::make_shared<AddressScraper>(
                resource.GetBGTileManager().ExtractMapBinary(resource.GetBGRoomBank().FilePath()));
            _rules = std::make_unique<ScraperScrollRuleProvider>(scraper);
            _renderer = std::make_unique<MapRenderer2D>(
                resource,
                owner.GetMapName(),
                owner.GetMapBinaryPath(),
                kTilePx);

            ScrollController::Params p;
            _scroll = std::make_unique<ScrollController>(*_rules, *_renderer, p);
            _scroll->SetPageIndex(0);
            _scroll->ObjectPos() = { 128.0, 120.0 };
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