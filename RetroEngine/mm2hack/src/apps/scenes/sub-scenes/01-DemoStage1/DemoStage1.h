//==============================================================================
// 
//  Project: mm2hack
//  DemoStage1.h
// 
//  Scene ID - 01 Demo stage scene.
// 
//==============================================================================
#pragma once

#include "apps/scenes/IBaseScene.h"

#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include "apps/graphics/bg/AddressScraper.h"
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/parameters/Parameters.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "apps/supervisor/ResourceManager.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::scenes
{
    // Identifiers for different phases within the BackdoorMenu
    enum class DemoStage1PhaseId : int
    {
        Main
    };

    class IDemoStage1Phase
    {
    public:
        virtual ~IDemoStage1Phase() = default;
        // Update the phase logic
        virtual void Update() = 0;
        // Render the phase-specific elements
        virtual void RenderWorld() = 0;
        // Render the phase-specific elements
        virtual void RenderOverlay() = 0;
        // Get owner phase id
        virtual DemoStage1PhaseId Id() const noexcept = 0;
        virtual void SetAddressScraper(std::unique_ptr<graphics::bg::AddressScraper> scraper) noexcept = 0;
    };

    // Demo stage scene (ID: 01)
    class DemoStage1 : public IBaseScene
    {
    public:
        explicit DemoStage1(SceneChangeMediator* mediator);
        ~DemoStage1() override;

        // === IBaseScene implementations ===
        // Initialize the backdoor menu
        void Initialize(const parameters::Parameters& params) override;
        // Main game logic execution
        void Update() override;
        // Render the world elements
        void RenderWorld() override;
        // Render the overlay elements
        void RenderOverlay() override;
        // Scene identification
        SceneID GetSceneID() const override { return SceneID::DemoStage1; }
        // Get the scene name (i.e. class name)
        std::wstring GetSceneName() const override { return kClassName; }
        // Get the map name used in this scene
        std::wstring GetMapName() const { return kMapName; }
        // Get the map binary path used in this scene
        std::wstring GetMapBinaryPath() const { return std::wstring(kStageMapBinary); }
        // Change child phase of the this scene
        void QueuePhase(std::unique_ptr<IDemoStage1Phase> next, PhaseFadePlan nextPlan);

        // === Save/Load state ===
        void Save(std::ostream& out);
        void Load(std::istream& in);

        // Access the fade controller for scene transitions
        PhaseFadeController& Fader() noexcept { return _fader; }

        auto& Input() noexcept { return _input; }
        const auto& Input() const noexcept { return _input; }

    private:
        bool InitializeResources(const parameters::Parameters& params);    // Initialize necessary resources
        void Finalize() override;       // Finalize the demo stage scene

    private:
        const std::wstring kClassName{ L"DemoStage1" };
        const std::wstring kMapName{ L"SAMPLESTAGE1" };
        const std::wstring_view kStageMapBinary{ L"assets\\mapdata\\multiway_test.bin" };

        struct RoomState {
            int pageIndex{ 0 };
            int tileW{ 8 };   // in tiles
            int tileH{ 8 };   // in tiles
        } _roomState;

        SceneChangeMediator* _mediator{ nullptr };              // Mediator for scene changes
        std::unique_ptr<IDemoStage1Phase> _phase;               // Current phase of the demo stage
        DemoStage1PhaseId _phaseId{ DemoStage1PhaseId::Main };  // Current phase identifier
        PhaseFadeController _fader;                             // Fade controller for scene transitions
        std::unique_ptr<IDemoStage1Phase> _pendingPhase;        // Pending phase to switch to
        PhaseFadePlan _pendingPlan{};                           // Pending fade plan for the next phase

        SceneID _nextScene{ SceneID::None };                    // Next scene to switch to
        parameters::Parameters _nextParams{};                   // Reserve the parameters for the next scene
        bool _leaving{ false };                                 // Flag indicating if leaving the backdoor menu

        const int fadeDurationFrames{ 16 };

        supervisor::ResourceManager* _resource{ nullptr };      // Reference to the resource manager
        core::assembly::StateProvider* _input{ nullptr };       // Reference to the state provider
        graphics::bg::BGTileManager::Id _bgTileId{
            static_cast<graphics::bg::BGTileManager::Id>(-1)
        };                                                      // Background tile set Id
    };
}