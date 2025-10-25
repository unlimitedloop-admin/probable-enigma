#include "pch.h"

#include "DemoStage1.h"

#include <cstdint>
#include <istream>
#include <ostream>
#include "apps/deal/GameContext.h"
#include "apps/graphics/bg/AddressScraper.h"
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/parameters/Parameters.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "config/GameAssets.h"
#include "DemoStage1Phase.h"
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
        using namespace apps::deal;
        auto& resource = GameContext::GetInstance().GetResourceManager();

        // Initialize the demo stage
        if (!InitializeResources(params))
        {
            THROW_EXCEPTION(L"Failed to initialize resources", kClassName);
        }

        _phase = std::make_unique<DemoStage1_::MainPhase>(*this);
        _phaseId = _phase->Id();

        PhaseFadePlan first(
            20, // preBlackHold
            20, // fadeInFrames
            0,  // preFadeOutHold
            12, // fadeOutFrames
            20, // postBlackHold
            FadeLayerMask::BG | FadeLayerMask::Font // layers
        );
        _fader.BeginPhase(first, resource);

        _resource = &resource;
        _input = &GameContext::GetInstance().Input();

        _phase->SetAddressScraper(std::make_unique<graphics::bg::AddressScraper>(
            resource.GetBGTileManager().ExtractMapBinary(
                resource.GetBGRoomBank().FilePath()
            )
        ));

        utils::debug_log(kClassName + L" initialized.");
    }

    bool DemoStage1::InitializeResources(const parameters::Parameters& params)
    {
        using namespace config;
        using namespace deal;
        auto& resource = GameContext::GetInstance().GetResourceManager();

        // Load the background tile graph.
        auto& bgTileManager = resource.GetBGTileManager();
        auto& bgRoomBank = resource.GetBGRoomBank();
        _bgTileId = bgTileManager.LoadTileset(kMapName, MM2H_GRAPHICS(SampleStage), MM2H_GRAPHPROPS(SampleStage));
        if (_bgTileId == graphics::bg::BGTileManager::Id(-1))
        {
            return false;
        }
        bgTileManager.SetMapSize(SystemConfig::kTileCountX, SystemConfig::kTileCountY);     // 16x15 tiles
        // Load map data.
        bgRoomBank.Load(kStageMapBinary);
        auto roomNo = params.Get<int>(L"RoomNo");
        if (auto idx = bgRoomBank.FindIndexByRoomId(static_cast<uint8_t>(*roomNo)); idx)
        {
            // Successfully found the room index.
            _roomState.pageIndex = static_cast<int>(*idx);
        }
        else
        {
            _roomState.pageIndex = 0; // Default to first page if not found.
        }
        bgTileManager.LoadMapBinary(bgRoomBank.FilePath(), bgRoomBank.PayloadOffset(static_cast<size_t>(_roomState.pageIndex)));

        const int bvmax = bgTileManager.VariantCountById(_bgTileId);
        bgTileManager.SetGlobalVariant(bvmax);
        resource.FadeInBG(fadeDurationFrames);

        return true;
    }

    void DemoStage1::Update()
    {
        using namespace apps::deal;
        auto& res = GameContext::GetInstance().GetResourceManager();

        // Proceed with fade process.
        _fader.Update(res);
        res.UpdateEffects();

        // Update with current phase logic, if not _phase then next scene.
        if (_phase)
        {
            _phase->Update();
        }
        else
        {
            if (_nextScene != SceneID::None && _mediator)
            {
                SceneID scene = _nextScene;
                parameters::Parameters params = std::move(_nextParams);

                // Clear members BEFORE calling mediator - don't touch 'this' after the call.
                _nextScene = SceneID::None;
                _nextParams = {}; // safe: done before potential destruction

                _mediator->RequestChange(scene, params);
            }
        }

        if (_fader.ReadyToSwitchPhase())
        {
            if (_pendingPhase)
            {
                _phase = std::move(_pendingPhase);
                _phaseId = _phase->Id();
                _fader.BeginPhase(_pendingPlan, res);
                _pendingPlan = {};
                utils::debug_log(kClassName + L" switched to new phase.");
            }
            else
            {
                // No pending phase, stay idle.
                _phase.reset();
                utils::debug_log(kClassName + L" has no pending phase, entering idle state.");
            }
        }
    }

    void DemoStage1::RenderWorld()
    {
        if (_phase) { _phase->RenderWorld(); }
    }

    void DemoStage1::RenderOverlay()
    {
        if (_phase) { _phase->RenderOverlay(); }
    }

    void DemoStage1::QueuePhase(std::unique_ptr<IDemoStage1Phase> next, PhaseFadePlan nextPlan)
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