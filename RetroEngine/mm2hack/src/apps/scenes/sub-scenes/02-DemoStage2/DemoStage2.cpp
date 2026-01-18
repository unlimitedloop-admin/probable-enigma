#include "pch.h"

#include "DemoStage2.h"

#include <istream>
#include <ostream>
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/resources/stages/StageTileAttributes.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/phases/ActionStageRuntimeBuilder.h"
#include "apps/scenes/phases/IPhase.h"
#include "apps/scenes/phases/PhaseResult.h"
#include "apps/scenes/phases/StageDefinition.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/systems/physics/TileAttribute.h"
#include "config/GameAssets.h"
#include "core/assembly/FilteredJoystickInputProvider.h"
#include "utils/output_debug.h"

#include "apps/scenes/phases/AbstractActionPhase.h"

namespace mm2hack::apps::scenes
{
    DemoStage2::DemoStage2(SceneChangeMediator* mediator)
        : _mediator(mediator)
    {
        utils::debug_log(kClassName + L" constructor called.");
    }

    DemoStage2::~DemoStage2()
    {
        // Clean up resources, finalize the demo stage, etc.
        utils::debug_log(kClassName + L" destructor called.");
    }

    void DemoStage2::Update()
    {
        using namespace apps::runtime;

        auto& res = GameContext::GetInstance().GetResourceManager();

        _fader.Update(res);
        res.UpdateEffects();

        // 1) First, handle fade switch (if ready, switch here)
        applyPendingPhaseIfReady_();

        // 2) Always update the phase (even during fade)
        if (_phase)
        {
            auto& gc = apps::runtime::GameContext::GetInstance();
            gc.FilteredJoystickInput().SetEnabled(_fader.InputEnabled());

            const phases::PhaseResult r = _phase->Update();
            if (r.HasRequest())
            {
                PhaseFadePlan defaultPlan(
                    5,   // preBlackHold
                    20,  // fadeInFrames
                    0,   // preFadeOutHold
                    20,  // fadeOutFrames
                    0,   // postBlackHold
                    FadeLayerMask::All // layers
                );

                Parameters empty;
                const auto* payload = (r.params != nullptr) ? r.params : &empty;

                dispatchTransition_(r.next_key, defaultPlan, *payload);
            }
        }
        else
        {
            // If there is no phase, proceed to the next scene.
            if (_nextScene != SceneID::None && _mediator)
            {
                SceneID scene = _nextScene;
                Parameters params = std::move(_nextParams);
                _nextScene = SceneID::None;
                _nextParams = {};
                _mediator->RequestChange(scene, params);
            }
        }
    }

    void DemoStage2::RenderWorld()
    {
        if (_phase) { _phase->RenderWorld(); }
    }

    void DemoStage2::RenderOverlay()
    {
        if (_phase) { _phase->RenderOverlay(); }
    }

    void DemoStage2::RequestTransition(const std::wstring& next_key, const PhaseFadePlan& plan, const Parameters& params)
    {
        // Phase asked for a transition; Scene decides what to build/queue.
        dispatchTransition_(next_key, plan, params);
    }

    void DemoStage2::QueuePhase(std::unique_ptr<phases::IPhase> next, PhaseFadePlan nextPlan)
    {
        _pendingPhase = std::move(next);
        _pendingPlan = nextPlan;

        if (_fader.Current() == PhaseFadeController::State::Interactive)
        {
            auto& res = apps::runtime::GameContext::GetInstance().GetResourceManager();
            _fader.RequestFadeOut(res);
        }
    }

    void DemoStage2::Save(std::ostream& out)
    {
        // Save the demo stage state
    }

    void DemoStage2::Load(std::istream& in)
    {
        // Load the demo stage state
    }

    void DemoStage2::onEnter_(const Parameters& params)
    {
        using namespace apps::runtime;
        auto& gc = GameContext::GetInstance();
        auto& resource = gc.GetResourceManager();

        // Initialize the demo stage
        if (!initializeResources_(params))
        {
            THROW_EXCEPTION(L"Failed to initialize resources", kClassName);
        }

        // Construct the Action Stage Runtime Context.
        // Create definition.
        phases::StageDefinition def{};
        def.map_binary_path = std::wstring(kStageMapBinary);
        def.start_page_index = _roomState.pageIndex;
        def.start_local_pos = { 128.0, 10.0 };

        // Create build config.
        phases::ActionStageBuildConfig cfg{};
        cfg.map_name = std::wstring(kMapName);
        cfg.tile_px = config::SystemConfig::kTileSize;
        cfg.player_sprite_id = static_cast<int>(_spriteId);

        // Create filtered joystick input provider.
        _input = &gc.Input();

        // Build the context using the builder.
        auto ctx = _actionBuilder.Build(resource, *_input, def, cfg, L"AreaA");

        // Create the initial phase.
        _phase = std::make_unique<phases::AbstractActionPhase>(std::move(ctx), _stageScript.get(), *this);

        PhaseFadePlan first(20, 20, 0, 12, 20, FadeLayerMask::All);
        _fader.BeginPhase(first, resource);

        _phase->Initialize(params);

        utils::debug_log(kClassName + L" initialized.");
    }

    void DemoStage2::onExit_()
    {
        using namespace runtime;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().ShutDown();
        if (_phase) _phase.reset();

        utils::debug_log(kClassName + L" finalized.");
    }

    bool DemoStage2::initializeResources_(const Parameters& params)
    {
        using namespace config;
        using namespace runtime;
        auto& resource = GameContext::GetInstance().GetResourceManager();

        // Load the background tile graph.
        auto& bgTileManager = resource.GetBGTileManager();
        auto& bgRoomBank = resource.GetBGRoomBank();
        _bgTileId = bgTileManager.LoadTileset(kMapName, MM2H_GRAPHICS(SampleStage), MM2H_GRAPHPROPS(SampleStage));
        if (_bgTileId == BGTileManagerId(-1))
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
        if (_spriteId == SpriteManagerId(-1))
        {
            return false;
        }
        const int sprvmax = spriteLoader.VariantCountById(_spriteId);
        spriteLoader.SetGlobalVariant(sprvmax);
        resource.FadeInSprite(fadeDurationFrames);

        return true;
    }

    void DemoStage2::loadStage_(rendering::bg::BGTileManager& bgTileManager)
    {
        using namespace systems::physics;
        using namespace resources::stages;
        ApplyTileAttributeRanges(bgTileManager, STAGE1_TILEATTRIBUTES);
    }

    void DemoStage2::applyPendingPhaseIfReady_()
    {
        using namespace apps::runtime;
        auto& res = GameContext::GetInstance().GetResourceManager();

        if (!_fader.ReadyToSwitchPhase())
        {
            return;
        }

        if (_pendingPhase)
        {
            _phase = std::move(_pendingPhase);
            _pendingPhase.reset();

            _fader.BeginPhase(_pendingPlan, res);
            _pendingPlan = {};

            // Phase Initialize after swap (optional).
            Parameters empty;
            _phase->Initialize(empty);
        }
        else
        {
            _phase.reset();
        }
    }

    void DemoStage2::dispatchTransition_(const std::wstring& next_key, const PhaseFadePlan& plan, const Parameters& params)
    {
        using namespace apps::runtime;

        // Example mapping.
        // Scene chooses which concrete phase to instantiate based on string keys.

        if (next_key == L"Intro")
        {
            // QueuePhase(std::make_unique<TopMenuPhase>(*this /* host */), plan);
            // ^ TopMenuPhase should take IPhaseHost& instead of DemoStage1&
            return;
        }

        if (next_key == L"AreaA" || next_key == L"AreaB")
        {
            auto& gc = GameContext::GetInstance();
            auto& res = gc.GetResourceManager();

            // StageDefinition minimal (your choice)
            phases::StageDefinition def{};
            def.map_binary_path = std::wstring(kStageMapBinary);
            def.start_page_index = _roomState.pageIndex; // or pick by key
            def.start_local_pos = { 128.0, 10.0 };

            phases::ActionStageBuildConfig cfg{};
            cfg.map_name = std::wstring(kMapName);
            cfg.tile_px = config::SystemConfig::kTileSize;
            cfg.player_sprite_id = static_cast<int>(_spriteId);

            auto ctx = _actionBuilder.Build(res, *_input, def, cfg, next_key);
            auto phase = std::make_unique<phases::AbstractActionPhase>(std::move(ctx), _stageScript.get(), *this /* host */);

            QueuePhase(std::move(phase), plan);
            return;
        }

        if (next_key == L"Clear")
        {
            // QueuePhase(std::make_unique<ClearPhase>(*this), plan);
            return;
        }

        // Unknown key -> ignore or log
        (void)params;
    }
}