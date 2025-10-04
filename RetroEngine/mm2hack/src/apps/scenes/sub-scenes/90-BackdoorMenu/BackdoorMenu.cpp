#include "pch.h"

#include "BackdoorMenu.h"

#include <istream>
#include <ostream>
#include "apps/deal/GameContext.h"
#include "apps/NES/NESPalette.h"
#include "apps/parameters/Parameters.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "BackdoorMenuPhase.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    BackdoorMenu::BackdoorMenu(SceneChangeMediator* mediator)
        : _mediator(mediator)
    {
        // Initialize the backdoor menu, load resources, etc.
        utils::debug_log(kClassName + L" constructor called.");
    }

    BackdoorMenu::~BackdoorMenu()
    {
        // Clean up resources, finalize the backdoor menu, etc.
        utils::debug_log(kClassName + L" destructor called.");
        Finalize();
    }

    void BackdoorMenu::Update()
    {
        if (_phase) { _phase->Update(); }
        else { _mediator->RequestChange(SceneID::None, {}); }
    }

    void BackdoorMenu::RenderWorld()
    {
        if (_phase) { _phase->RenderWorld(); }
    }

    void BackdoorMenu::RenderOverlay()
    {
        if (_phase) { _phase->RenderOverlay(); }
    }

    void BackdoorMenu::SetPhase(std::unique_ptr<IBackdoorMenuPhase> next)
    {
        _phase = std::move(next);
    }

    void BackdoorMenu::Save(std::ostream& out)
    {
        // TODO: Save scene state to a file
        int phase = static_cast<int>(_phaseId);
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
        _phaseId = static_cast<PhaseId>(phase);
        // Load other necessary data
        // e.g., background stars, current selection, etc.
        _starField.Load(in);    // Load background stars state
    }

    void BackdoorMenu::Initialize(const parameters::Parameters& params)
    {
        utils::debug_log(kClassName + L" initialized.");

        using namespace apps::deal;
        auto& resource = GameContext::GetInstance().GetResourceManager();
        auto& font = resource.GetFontTileManager();
        font.SetUp();

        NES::NESPalette::SetBackgroundFor(13U); // Innocent black

        const int vmax = std::max(0, font.MaxVariant());
        font.SetGlobalVariant(vmax);    // Max contrast
        resource.FadeInFont(20);

        _phaseId = PhaseId::Credit;
        _phase = MakePhase(_phaseId, *this);
        _starField.InitStars();
        _resource = GameContext::GetInstance().GetResourceManagerPtr();
        _input = &GameContext::GetInstance().Input();
    }

    void BackdoorMenu::Finalize()
    {
        using namespace apps::deal;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().ShutDown();
        _phase.reset();

        utils::debug_log(kClassName + L" finalized.");
    }

    std::unique_ptr<IBackdoorMenuPhase> BackdoorMenu::MakePhase(PhaseId id, BackdoorMenu& owner)
    {
        using namespace BackdoorMenu_;
        switch (id)
        {
        case PhaseId::Credit:     return std::make_unique<CreditPhase>(owner);
        case PhaseId::TopMenu:    return std::make_unique<TopMenuPhase>(owner);
        case PhaseId::InsideMenu: return std::make_unique<InsideMenuPhase>(owner);
        }
        // Safety fallback
        return std::make_unique<CreditPhase>(owner);
    }
}