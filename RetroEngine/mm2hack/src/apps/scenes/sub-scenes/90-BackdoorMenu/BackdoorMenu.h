//==============================================================================
// 
//  Project: mm2hack
//  BackdoorMenu.h
// 
//  Scene ID - 90 BackdoorMenu scene.
// 
//==============================================================================
#pragma once

#include "apps/scenes/IBaseScene.h"

#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include "apps/parameters/Parameters.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "apps/supervisor/ResourceManager.h"
#include "apps/vfx/stareffects/BgStarField.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::scenes
{
    // Interface for different phases of the BackdoorMenu class
    class IBackdoorMenuPhase
    {
    public:
        virtual ~IBackdoorMenuPhase() = default;
        // Update the phase logic
        virtual void Update() = 0;
        // Render the phase-specific elements
        virtual void RenderWorld() = 0;
        // Render the phase-specific elements
        virtual void RenderOverlay() = 0;
        // Check if the phase is complete and ready to transition
        virtual bool IsComplete() const = 0;
    };

    // Backdoor menu for debugging purposes only (A list of selectable scenes)
    class BackdoorMenu : public IBaseScene
    {
    public:
        BackdoorMenu(SceneChangeMediator* mediator);
        ~BackdoorMenu() override;

        // === IBaseScene implementations ===
        // Main game logic execution
        void Update() override;
        // Render the game world
        void RenderWorld() override;
        // Render overlays (e.g., HUD, menus)
        void RenderOverlay() override;
        // Scene identification
        SceneID GetSceneID() const override { return SceneID::BackdoorMenu; }
        // Get the scene name (i.e. class name)
        std::wstring GetSceneName() const override { return kClassName; }
        // Change child phase of the this scene
        void SetPhase(std::unique_ptr<IBackdoorMenuPhase> next);

        void Save(std::ostream& out);
        void Load(std::istream& in);

        enum class PhaseId : int
        {
            Credit,
            TopMenu,
            InsideMenu
        };

        auto& StarField() noexcept { return _starField; }
        const auto& StarField() const noexcept { return _starField; }
        auto& Resource() noexcept { return _resource; }
        const auto& Resource() const noexcept { return _resource; }
        auto& Input() noexcept { return _input; }
        const auto& Input() const noexcept { return _input; }

        PhaseId CurrentPhase() const noexcept { return _phaseId; }

        std::unique_ptr<IBackdoorMenuPhase> MakePhase(PhaseId id, BackdoorMenu& owner);

    private:
        void Initialize(const parameters::Parameters& params) override; // Initialize the backdoor menu
        void Finalize() override;                                       // Finalize the backdoor menu

    private:
        const std::wstring kClassName = L"BackdoorMenu";

        SceneChangeMediator* _mediator{ nullptr };          // Mediator for scene changes
        std::unique_ptr<IBackdoorMenuPhase> _phase;         // Current phase of the backdoorMenu
        PhaseId _phaseId{ PhaseId::Credit };                // Current phase identifier

        vfx::stareffects::BgStarField _starField;           // Background star field effect
        supervisor::ResourceManager* _resource{ nullptr };  // Reference to the resource manager
        core::assembly::StateProvider* _input{ nullptr };   // Reference to the state provider
    };
}