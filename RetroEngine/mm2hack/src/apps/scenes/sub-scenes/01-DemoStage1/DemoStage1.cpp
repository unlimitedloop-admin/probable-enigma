#include "pch.h"

#include "DemoStage1.h"

#include <istream>
#include <ostream>
#include "apps/deal/GameContext.h"
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/parameters/Parameters.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "config/GameAssets.h"
#include "exceptions/CoreException.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    DemoStage1::DemoStage1(SceneChangeMediator* mediator)
        : _mediator(mediator)
    {
        utils::debug_log(kClassName + L" constructor called.");
    }

    DemoStage1::~DemoStage1()
    {
        // Clean up resources, finalize the demo stage, etc.
        utils::debug_log(kClassName + L" destructor called.");
        Finalize();
    }

    void DemoStage1::Initialize(const parameters::Parameters& params)
    {
        // Initialize the demo stage
        if (!InitializeResources())
        {
            THROW_EXCEPTION(L"Failed to initialize resources", kClassName);
        }
    }

    bool DemoStage1::InitializeResources()
    {
        using namespace config;
        using namespace deal;
        auto& resource = GameContext::GetInstance().GetResourceManager();

        // Load the background tile graph.
        auto& bgTileManager = resource.GetBGTileManager();
        _bgTileId = bgTileManager.LoadTileset(kMapName, MM2H_GRAPHICS(SampleStage), MM2H_GRAPHPROPS(SampleStage));
        if (_bgTileId == graphics::bg::BGTileManager::Id(-1))
        {
            return false;
        }
        bgTileManager.SetMapSize(SystemConfig::kTileCountX, SystemConfig::kTileCountY);     // 16x15 tiles
        // Load map data.
        bgTileManager.LoadMapBinary(kStageMapBinary, 0x10);
        const int bvmax = bgTileManager.VariantCountById(_bgTileId);
        bgTileManager.SetGlobalVariant(bvmax);
        resource.FadeInBG(fadeDurationFrames);

        return true;
    }

    void DemoStage1::Update()
    {
        // Update the demo stage
        using namespace deal;
        auto& resource = GameContext::GetInstance().GetResourceManager();
        resource.UpdateEffects();   // Update fade effects
    }

    void DemoStage1::RenderWorld()
    {
        // Render the world elements
        using namespace config;
        using namespace deal;
        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Draw the graph from the resource manager.
        bgTileManager.DrawMapByName(kMapName, SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight, 0, 0);
    }

    void DemoStage1::RenderOverlay()
    {
        // Render the overlay elements
    }

    void DemoStage1::QueuePhase(std::unique_ptr<IDemoStagePhase> next, PhaseFadePlan nextPlan)
    {
        _pendingPhase = std::move(next);
        _pendingPlan = nextPlan;
    }

    void DemoStage1::Save(std::ostream& out)
    {
        // Save the demo stage state
    }

    void DemoStage1::Load(std::istream& in)
    {
        // Load the demo stage state
    }

    void DemoStage1::Finalize()
    {
        using namespace apps::deal;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().ShutDown();
        _phase.reset();

        utils::debug_log(kClassName + L" finalized.");
    }
}