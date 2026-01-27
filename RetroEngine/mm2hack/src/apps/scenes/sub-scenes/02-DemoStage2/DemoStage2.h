//==============================================================================
// 
//  Project: mm2hack
//  DemoStage2.h
// 
//  Scene ID - 02 Demo stage 2 scene implemented with abstract action scene.
// 
//==============================================================================
#pragma once

#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/phases/IPhaseHost.h"

#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/resources/ResourceManager.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/phases/ActionStageRuntimeBuilder.h"
#include "apps/scenes/phases/IPhase.h"
#include "apps/scenes/phases/IStageScript.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::scenes
{
    // Demo stage scene (ID: 02)
    class DemoStage2 final : public IBaseScene, public phases::IPhaseHost
    {
        using BGTileManagerId       = rendering::bg::BGTileManager::Id;
        using SpriteManagerId       = rendering::sprite::SpriteManager::Id;
        using Parameters            = resources::parameters::Parameters;

    public:
        explicit DemoStage2(SceneChangeMediator* mediator);
        ~DemoStage2() override;

        // === IBaseScene implementations ===
        // Main update loop
        void Update() override;
        // Render world elements
        void RenderWorld() override;
        // Render overlay elements
        void RenderOverlay() override;
        // Scene identification
        SceneID GetSceneID() const override { return SceneID::DemoStage2; }
        // Get the scene name (i.e. class name)
        std::wstring GetSceneName() const override { return kClassName; }

        // === IPhaseHost implementations ===
        // Request a phase transition
        void RequestTransition(const std::wstring& next_key, const PhaseFadePlan& plan, const Parameters& params) override;

        // === DemoStage2 specific ===
        // Queue a new phase to transition to
        void QueuePhase(std::unique_ptr<phases::IPhase> next, PhaseFadePlan nextPlan);
        // Get the current room page index
        int GetCurrentRoomPageIndex() const { return _roomState.pageIndex; }
        // Get the map name used in this scene
        std::wstring GetMapName() const { return kMapName; }
        // Get the map binary path used in this scene
        std::wstring GetMapBinaryPath() const { return std::wstring(kStageMapBinary); }
        // Get the sprite Id used in this scene
        SpriteManagerId GetSpriteId() const noexcept { return _spriteId; }

        // === Save/Load state ===
        // Save the current state to an output stream
        void Save(std::ostream& out);
        // Load the state from an input stream
        void Load(std::istream& in);

    private:
        void onEnter_(const Parameters& params) override;               // Scene enter hook
        void onExit_() override;                                        // Scene exit hook

        bool initializeResources_(const Parameters& params);            // Initialize resources needed for the scene
        void loadStage_(rendering::bg::BGTileManager& bgTileManager);   // Load the stage map and tile attributes

        void applyPendingPhaseIfReady_();                               // Apply pending phase if fader is ready
        void dispatchTransition_(
            const std::wstring& next_key,
            const PhaseFadePlan& plan, const Parameters& params);       // Dispatch phase transition request

    private:
        const std::wstring kClassName{ L"DemoStage2" };

        const std::wstring kMapName{ L"SAMPLESTAGE2" };
        const std::wstring_view kStageMapBinary{ L"assets\\_exams\\bg\\SAMPLESTAGE1.bin" };
        
        struct RoomState
        {
            int pageIndex{ 0 };
            int tileW{ 8 };   // in tiles
            int tileH{ 8 };   // in tiles
        } _roomState;                                                   // Current room state is used to track room/page info

        SceneChangeMediator* _mediator{ nullptr };                      // Mediator for scene changes

        std::unique_ptr<phases::IPhase> _phase{};                       // Current active phase
        std::unique_ptr<phases::IPhase> _pendingPhase{};                // Pending phase to switch to
        PhaseFadePlan _pendingPlan{};                                   // Fade plan for the pending phase

        PhaseFadeController _fader{};                                   // Fade controller for transitions
        const int fadeDurationFrames{ 16 };                             // Duration of fade in frames

        SceneID _nextScene{ SceneID::None };                            // Next scene to switch to
        Parameters _nextParams{};                                       // Reserve the parameters for the next scene

        core::assembly::StateProvider* _input{};                        // Input state provider
        resources::ResourceManager* _resource{};                        // Resource manager
        phases::ActionStageRuntimeBuilder _actionBuilder{};             // Builder for action stage runtime
        std::unique_ptr<phases::IStageScript> _stageScript{};           // Stage script

        BGTileManagerId _bgTileId{ static_cast<BGTileManagerId>(-1) };  // Background tile set Id
        SpriteManagerId _spriteId{ static_cast<SpriteManagerId>(-1) };  // Sprite set Id
    };
}