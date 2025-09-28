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
#include <map>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include "apps/parameters/Parameters.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "apps/vfx/stareffects/BgStarField.h"

namespace mm2hack::apps::scenes
{
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

        void Save(std::ostream& out);
        void Load(std::istream& in);

    private:
        void Initialize(const parameters::Parameters& params) override; // Initialize the backdoor menu
        void Finalize() override;                                       // Finalize the backdoor menu

    private:
        const std::wstring kClassName = L"BackdoorMenu";

        enum class Phase
        {
            Credit,
            TopMenu,
            InsideMenu,
            End,
        };

        using PhaseHandler = void (BackdoorMenu::*)();
        std::map<Phase, PhaseHandler> _phaseHandlers;
        Phase _currentPhase{ Phase::Credit };

        int _cursorPos{ 0 };
        std::vector<std::pair<std::wstring, SceneID>> _menuItems;

        void HandleCreditPhase();
        void HandleTopMenuPhase();
        void HandleInsideMenuPhase();

        SceneChangeMediator* _mediator{ nullptr };      // Mediator for scene changes

        vfx::stareffects::BgStarField _starField;       // Background star field effect
    };
}