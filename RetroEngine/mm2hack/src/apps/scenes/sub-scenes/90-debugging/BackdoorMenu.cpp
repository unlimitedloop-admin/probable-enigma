#include "pch.h"

#include "BackdoorMenu.h"

#include <istream>
#include <map>
#include <ostream>
#include "apps/deal/GameContext.h"
#include "apps/NES/NESPalette.h"
#include "apps/parameters/Parameters.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "input/Jpbtn.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    BackdoorMenu::BackdoorMenu(SceneChangeMediator* mediator)
        : _mediator(mediator)
    {
        // Initialize the backdoor menu, load resources, etc.
        utils::debug_log(kClassName + L" constructor called.");

        _phaseHandlers = {
            { Phase::Credit, &BackdoorMenu::HandleCreditPhase },
            { Phase::TopMenu, &BackdoorMenu::HandleTopMenuPhase },
            { Phase::InsideMenu, &BackdoorMenu::HandleInsideMenuPhase },
        };
    }

    BackdoorMenu::~BackdoorMenu()
    {
        // Clean up resources, finalize the backdoor menu, etc.
        utils::debug_log(kClassName + L" destructor called.");
        Finalize();
    }

    void BackdoorMenu::Update()
    {
        // Backdoor menu main game logic execution
        if (auto it = _phaseHandlers.find(_currentPhase); it != _phaseHandlers.end())
        {
            (this->*it->second)();  // Call the appropriate phase handler
        }
        else
        {
            // Next Scene
            _mediator->RequestChange(SceneID::None, {});
        }
    }

    void BackdoorMenu::RenderWorld()
    {
        // Render the game world for the backdoor menu
    }

    void BackdoorMenu::RenderOverlay()
    {
        // Render any overlays for the backdoor menu
    }

    void BackdoorMenu::Save(std::ostream& out)
    {
        // TODO: Save scene state to a file
        int phase = static_cast<int>(_currentPhase);
        out.write(reinterpret_cast<const char*>(&phase), sizeof(phase));
        // Save other necessary data
        // e.g., background stars, current selection, etc.
        _starField.Save(out);   // Save background stars state
    }

    void BackdoorMenu::Load(std::istream& in)
    {
        // TODO: Load scene state from a file
        int phase = 0;
        in.read(reinterpret_cast<char*>(&phase), sizeof(phase));
        _currentPhase = static_cast<Phase>(phase);
        // Load other necessary data
        // e.g., background stars, current selection, etc.
        _starField.Load(in);    // Load background stars state
    }

    void BackdoorMenu::Initialize(const parameters::Parameters& params)
    {
        utils::debug_log(kClassName + L" initialized.");

        using namespace apps::deal;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().SetUp();

        NES::NESPalette::SetBackgroundFor(13U); // Innocent black

        _currentPhase = Phase::Credit;
        _starField.InitStars();
    }

    void BackdoorMenu::Finalize()
    {
        utils::debug_log(kClassName + L" finalized.");

        using namespace apps::deal;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().Shutdown();
    }

    void BackdoorMenu::HandleCreditPhase()
    {
        // Handle the credit phase logic
        using namespace apps::deal;
        using namespace core::assembly;

        auto& input = GameContext::GetInstance().Input();
        auto startPressed = input.JustPressed(JPBTN::START) || input.JustPressed(JPBTN::A);
        auto& fonts = GameContext::GetInstance().GetResourceManager().GetFontTileManager();
        fonts.DrawTextImage(L"BACKDOOR MENU", 76, 56);
        fonts.DrawTextImage(L" PRESS START ", 76, 96);

        if (startPressed)
        {
            _currentPhase = Phase::TopMenu; // Transition to the next phase for demonstration
        }
    }

    void BackdoorMenu::HandleTopMenuPhase()
    {
    }

    void BackdoorMenu::HandleInsideMenuPhase()
    {
    }
}