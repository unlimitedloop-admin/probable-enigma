#include "pch.h"

#include "BackdoorMenu.h"

#include <istream>
#include <ostream>
#include "apps/deal/GameContext.h"
#include "apps/NES/NESPalette.h"
#include "apps/parameters/Parameters.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/scenes/SceneID.h"
#include "BackdoorMenuPhase.h"
#include "config/PathDefsJsonProps.h"
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
        using namespace apps::deal;
        auto& res = GameContext::GetInstance().GetResourceManager();

        // Proceed with fade process.
        _fader.Update(res);
        res.UpdateEffects();

        // Update with current phase logic, if not _phase then next scene.
        if (_phase) { _phase->Update(); }
        else { _mediator->RequestChange(SceneID::None, {}); }   // TODO: Set next scene ID. (optional parameters)

        if (_pendingPhase && _fader.ReadyToSwitchPhase())
        {
            _phase = std::move(_pendingPhase);
            _phaseId = _phase->Id();
            _fader.BeginPhase(_pendingPlan, res);
            _pendingPlan = {};
            utils::debug_log(kClassName + L" switched to new phase.");
        }
    }

    void BackdoorMenu::RenderWorld()
    {
        if (_phase) { _phase->RenderWorld(); }
    }

    void BackdoorMenu::RenderOverlay()
    {
        if (_phase) { _phase->RenderOverlay(); }
    }

    void BackdoorMenu::QueuePhase(std::unique_ptr<IBackdoorMenuPhase> next, PhaseFadePlan nextPlan)
    {
        // NOTE: A method that actually has the function of switching phases.
        _pendingPhase = std::move(next);
        _pendingPlan = nextPlan;

        if (_fader.Current() == PhaseFadeController::State::Interactive)
        {
            if (_resource != nullptr) {
                _fader.RequestFadeOut(*_resource);
            }
        }
        // Already fading out or in transition, will switch when ready.
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

        auto& audio = resource.GetAudioManager();
        audio.Initialize(MM2H_PROPERTY(BackdoorMenuSoundProperty));

        NES::NESPalette::SetBackgroundFor(13U); // Innocent black

        _phase = std::make_unique<BackdoorMenu_::CreditPhase>(*this);
        _phaseId = _phase->Id();

        PhaseFadePlan first(
            20, // preBlackHold
            20, // fadeInFrames
            0,  // preFadeOutHold
            12, // fadeOutFrames
            20,  // postBlackHold
            FadeLayerMask::BG | FadeLayerMask::Font // layers
        );
        _fader.BeginPhase(first, resource);

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

    // TODO: Check if this is used anywhere, if not, remove it.
    //std::unique_ptr<IBackdoorMenuPhase> BackdoorMenu::MakePhase(PhaseId id, BackdoorMenu& owner)
    //{
    //    using namespace BackdoorMenu_;
    //    switch (id)
    //    {
    //    case PhaseId::Credit:     return std::make_unique<CreditPhase>(owner);
    //    case PhaseId::TopMenu:    return std::make_unique<TopMenuPhase>(owner);
    //    case PhaseId::InsideMenu: return std::make_unique<InsideMenuPhase>(owner);
    //    }
    //    // Safety fallback
    //    return std::make_unique<CreditPhase>(owner);
    //}
}