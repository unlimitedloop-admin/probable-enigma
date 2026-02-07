#include "pch.h"

#include "DemoStage2.h"

#include <istream>
#include <ostream>
#include "apps/foundation/NES/NESPalette.h"
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/resources/stages/StageTileAttributes.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/phases/AbstractActionPhase.h"
#include "apps/scenes/phases/ActionStageRuntimeBuilder.h"
#include "apps/scenes/phases/IPhase.h"
#include "apps/scenes/phases/PhaseResult.h"
#include "apps/scenes/phases/StageDefinition.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"
#include "config/GameAssets.h"
#include "config/PathDefsJsonProps.h"
#include "core/assembly/FilteredJoystickInputProvider.h"
#include "utils/output_debug.h"

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

            auto* audio = &gc.GetResourceManager().GetAudioManager();
            audio->Update();

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

    bool DemoStage2::TryEnemySprite(world::entity::enemy::EnemyKind kind, SpriteManagerId& out) const noexcept
    {
        const auto it = _spriteBank.enemies.find(kind);
        if (it == _spriteBank.enemies.end())
        {
            return false;
        }
        out = it->second;
        return true;
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
        phases::ActionStageBuildConfig build{};
        build.map_name = std::wstring(kMapName);
        build.tile_px = config::SystemConfig::kTileSize;
        build.asset_provider = this;

        // Create filtered joystick input provider.
        _input = &gc.Input();
        // Build the context using the builder.
        auto ctx = _actionBuilder.Build(resource, *_input, def, build, L"AreaA");
        // Create the initial phase.
        _phase = std::make_unique<phases::AbstractActionPhase>(std::move(ctx), _stageScript.get(), *this);

        // Start with a fade-in.
        PhaseFadePlan first(
            20,     // preBlackHold
            20,     // fadeInFrames
            0,      // preFadeOutHold
            12,     // fadeOutFrames
            20,     // postFadeOutHold
            FadeLayerMask::All  // layers
        );
        _fader.BeginPhase(first, resource);

        _phase->Initialize(params);

        utils::debug_log(kClassName + L" initialized.");
    }

    void DemoStage2::onExit_()
    {
        using namespace runtime;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().ShutDown();
        GameContext::GetInstance().GetResourceManager().GetAudioManager().Release();
        if (_phase) _phase.reset();

        utils::debug_log(kClassName + L" finalized.");
    }

    bool DemoStage2::initializeResources_(const Parameters& params)
    {
        using namespace config;
        using namespace runtime;
        auto& resource = GameContext::GetInstance().GetResourceManager();

        auto& audio = resource.GetAudioManager();
        if (!audio.Initialize(MM2H_PROPERTY(DemoStage2SoundProperty)))
        {
            return false;
        }

        foundation::NES::NESPalette::SetBackgroundFor(13U); // Innocent black

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
        resource.FadeInBG(_fadeDurationFrames);

        // Load the graph from the resource manager.
        loadAssets_();

        return true;
    }

    void DemoStage2::loadStage_(rendering::bg::BGTileManager& bgTileManager)
    {
        using namespace systems::physics;
        using namespace resources::stages;
        ApplyTileAttributeRanges(bgTileManager, STAGE1_TILEATTRIBUTES);
    }

    bool DemoStage2::loadAssets_()
    {
        auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
        auto& spriteLoader = resource.GetSpriteManager();

        _spriteBank.player = spriteLoader.Load(L"Player", MM2H_GRAPHICS(Player), MM2H_GRAPHPROPS(Player));
        if (_spriteBank.player == SpriteManagerId(-1)) return false;

        _spriteBank.player_attack = spriteLoader.Load(L"PlayerAttack", MM2H_GRAPHICS(PlayerEquip), MM2H_GRAPHPROPS(PlayerEquip));
        if (_spriteBank.player_attack == SpriteManagerId(-1)) return false;

        const int sprvmax = spriteLoader.VariantCountById(_spriteBank.player);
        spriteLoader.SetGlobalVariant(sprvmax);
        resource.FadeInSprite(_fadeDurationFrames);
        return true;
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

            phases::ActionStageBuildConfig build{};
            build.map_name = std::wstring(kMapName);
            build.tile_px = config::SystemConfig::kTileSize;
            build.asset_provider = this;

            auto ctx = _actionBuilder.Build(res, *_input, def, build, next_key);

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