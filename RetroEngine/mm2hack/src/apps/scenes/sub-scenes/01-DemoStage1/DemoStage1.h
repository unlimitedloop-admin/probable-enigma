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
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/SceneID.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/ITileMapProvider.h"

namespace mm2hack::apps
{
    namespace rendering::bg
    {
        class BGTileManager;
    }

    namespace resources
    {
        namespace parameters
        {
            class Parameters;
        }
        class ResourceManager;
    }

    namespace runtime
    {
        class GameContext;
    }

    namespace scenes
    {
        class SceneChangeMediator;
    }
}

namespace mm2hack::core::assembly
{
    class StateProvider;
}

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
        // Initialize the phase
        virtual void Initialize() = 0;
        // Update the phase logic
        virtual void Update() = 0;
        // Render the phase-specific elements
        virtual void RenderWorld() = 0;
        // Render the phase-specific elements
        virtual void RenderOverlay() = 0;
        // Get owner phase id
        virtual DemoStage1PhaseId Id() const noexcept = 0;
    };

    // Demo stage scene (ID: 01)
    class DemoStage1 : public IBaseScene
    {
        using ResourceManager   = apps::resources::ResourceManager;
        using StateProvider     = core::assembly::StateProvider;
        using BGTileManagerId   = rendering::bg::BGTileManager::Id;
        using Parameters        = resources::parameters::Parameters;
        using ITerrainProbe     = systems::physics::ITerrainProbe;
        using ITileMapProvider  = systems::physics::ITileMapProvider;

    public:
        explicit DemoStage1(SceneChangeMediator* mediator);
        ~DemoStage1() override;

        // === IBaseScene implementations ===
        // Initialize the backdoor menu
        void Initialize(const Parameters& params) override;
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

        auto& ResourceManagerObj() noexcept { return *_resource; }
        const auto& ResourceManagerPtr() const noexcept { return *_resource; }

        auto& Input() noexcept { return _input; }
        const auto& Input() const noexcept { return _input; }

    private:
        bool initializeResources_(const Parameters& params);    // Initialize necessary resources
        void finalize_() override;       // Finalize the demo stage scene

    private:
        const std::wstring kClassName{ L"DemoStage1" };
        const std::wstring kMapName{ L"SAMPLESTAGE3" };
        const std::wstring_view kStageMapBinary{ L"assets\\_exams\\bg\\DEMOSTAGE3.bin" };

        struct RoomState {
            int pageIndex{ 0 };
            int tileW{ 8 };   // in tiles
            int tileH{ 8 };   // in tiles
        } _roomState;

        SceneChangeMediator* _mediator{ nullptr };                      // Mediator for scene changes
        std::unique_ptr<IDemoStage1Phase> _phase;                       // Current phase of the demo stage
        DemoStage1PhaseId _phaseId{ DemoStage1PhaseId::Main };          // Current phase identifier
        PhaseFadeController _fader;                                     // Fade controller for scene transitions
        std::unique_ptr<IDemoStage1Phase> _pendingPhase;                // Pending phase to switch to
        PhaseFadePlan _pendingPlan{};                                   // Pending fade plan for the next phase

        SceneID _nextScene{ SceneID::None };                            // Next scene to switch to
        Parameters _nextParams{};                                       // Reserve the parameters for the next scene
        bool _leaving{ false };                                         // Flag indicating if leaving the backdoor menu

        const int fadeDurationFrames{ 16 };

        ResourceManager* _resource{ nullptr };                          // Reference to the resource manager
        StateProvider* _input{ nullptr };                               // Reference to the state provider
        BGTileManagerId _bgTileId{ static_cast<BGTileManagerId>(-1) };  // Background tile set Id
    };
}