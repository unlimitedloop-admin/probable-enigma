#include "pch.h"

#include "DemoStage1.h"

#include <istream>
#include <ostream>
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/resources/parameters/Parameters.h"
#include "apps/resources/stages/StageTileAttributes.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "apps/systems/physics/TileAttribute.h"
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
        finalize_();
    }

    void DemoStage1::Initialize(const Parameters& params)
    {
        using namespace apps::runtime;
        auto& resource = GameContext::GetInstance().GetResourceManager();

        // Initialize the demo stage
        if (!initializeResources_(params))
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
            FadeLayerMask::All // layers
        );
        _fader.BeginPhase(first, resource);

        _input = &GameContext::GetInstance().Input();

        _phase->Initialize();

        utils::debug_log(kClassName + L" initialized.");
    }

    bool DemoStage1::initializeResources_(const Parameters& params)
    {
        using namespace config;
        using namespace runtime;
        auto& resource = GameContext::GetInstance().GetResourceManager();

        // Load the background tile graph.
        auto& bgTileManager = resource.GetBGTileManager();
        auto& bgRoomBank = resource.GetBGRoomBank();
        _bgTileId = bgTileManager.LoadTileset(kMapName, MM2H_GRAPHICS(SampleStage), MM2H_GRAPHPROPS(SampleStage));
        if (_bgTileId == rendering::bg::BGTileManager::Id(-1))
        {
            return false;
        }
        bgTileManager.SetMapSize(SystemConfig::kTileCountX, SystemConfig::kTileCountY);     // 16x15 tiles
        // Load map data.
        bgRoomBank.Load(kStageMapBinary);
        auto roomNo = params.Get<int>(L"RoomNo");
        if (roomNo == std::nullopt)
        {
            roomNo = 0; // Default to room 0 if not specified.
        }
        if (auto idx = bgRoomBank.FindIndexByRoomId(static_cast<uint8_t>(*roomNo)); idx)
        {
            // Successfully found the room index.
            _roomState.pageIndex = static_cast<int>(*idx);
        }
        else
        {
            _roomState.pageIndex = 0; // Default to first page if not found.
        }
        loadStage_(bgTileManager);

        const int bvmax = bgTileManager.VariantCountById(_bgTileId);
        bgTileManager.SetGlobalVariant(bvmax);
        resource.FadeInBG(fadeDurationFrames);

        // Load the graph from the resource manager.
        auto& spriteLoader = resource.GetSpriteManager();
        _spriteId = spriteLoader.Load(L"Player", MM2H_GRAPHICS(Player), MM2H_GRAPHPROPS(Player));
        if (_spriteId == rendering::sprite::SpriteManager::Id(-1))
        {
            return false;
        }
        const int sprvmax = spriteLoader.VariantCountById(_spriteId);
        spriteLoader.SetGlobalVariant(sprvmax);
        resource.FadeInSprite(fadeDurationFrames);

        return true;
    }

    void DemoStage1::loadStage_(BGTileManager& bgTileManager)
    {
        using namespace systems::physics;
        using namespace resources::stages;
        ApplyTileAttributeRanges(bgTileManager, STAGE1_TILEATTRIBUTES);
    }

    void DemoStage1::Update()
    {
        auto& resource = runtime::GameContext::GetInstance().GetResourceManager();

        // Proceed with fade process.
        _fader.Update(resource);
        resource.UpdateEffects();

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
                Parameters params = std::move(_nextParams);

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
                _fader.BeginPhase(_pendingPlan, resource);
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
        //if (_phase) { _phase->RenderOverlay(); }
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

    void DemoStage1::finalize_()
    {
        using namespace runtime;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().ShutDown();
        if (_phase) _phase.reset();

        utils::debug_log(kClassName + L" finalized.");
    }
}