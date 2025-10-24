#include "pch.h"

#include "DemoStage1Phase.h"

#include "apps/deal/GameContext.h"
#include "DemoStage1.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        //============================================================================== 
        //
        //  MainPhase
        //
        //==============================================================================
        void MainPhase::Update()
        {
            if (!owner.Fader().InputEnabled()) return;
        }

        void MainPhase::RenderWorld()
        {
            // Render the world elements
            using namespace config;
            using namespace deal;
            auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

            // Draw the graph from the resource manager.
            bgTileManager.DrawMapByName(owner.GetMapName(), SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight, 0, 0);
        }

        void MainPhase::RenderOverlay()
        {
            // Render the overlay elements for the demo stage here.
        }

        DemoStage1PhaseId MainPhase::Id() const noexcept
        {
            return DemoStage1PhaseId::Main;
        }
    }
}