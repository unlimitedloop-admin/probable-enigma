#include "pch.h"

#include "BackdoorMenu.h"

#include <istream>
#include <iterator>
#include <ostream>
#include "apps/foundation/NES/NESPalette.h"
#include "apps/resources/parameters/Parameters.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/scenes/SceneChangeMediator.h"
#include "apps/vfx/cursor/TwinkleCursorAnimator.h"
#include "BackdoorMenuPhase.h"
#include "config/GameAssets.h"
#include "config/PathDefsJsonProps.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::scenes
{
    BackdoorMenu::BackdoorMenu(SceneChangeMediator* mediator)
        : _mediator(mediator)
        , _cursor(runtime::GameContext::GetInstance().GetResourceManager().GetSpriteManager(), 16, 16)
    {
        utils::debug_log(kClassName + L" constructor called.");
    }

    BackdoorMenu::~BackdoorMenu()
    {
        // Clean up resources, finalize the backdoor menu, etc.
        utils::debug_log(kClassName + L" destructor called.");
    }

    void BackdoorMenu::Update()
    {
        using namespace apps::runtime;
        auto& res = GameContext::GetInstance().GetResourceManager();

        // Proceed with fade process.
        _fader.Update(res);
        res.UpdateEffects();

        // Update with current phase logic, if not _phase then next scene.
        if (_phase)
        {
            _phase->Update();
        }
        else
        {
            if (_nextScene != SceneID::None && _mediator)
            {
                SceneID scene = _nextScene;
                Parameters params = std::move(_nextParams);

                // Clear members BEFORE calling mediator - don't touch 'this' after the call.
                _nextScene = SceneID::None;
                _nextParams = {}; // safe: done before potential destruction

                _mediator->RequestChange(scene, params);
            }
        }

        if (_fader.ReadyToSwitchPhase())
        {
            if (_pendingPhase)
            {
                _phase = std::move(_pendingPhase);
                _phaseId = _phase->Id();
                _fader.BeginPhase(_pendingPlan, res);
                _pendingPlan = {};
                utils::debug_log(kClassName + L" switched to new phase.");
            }
            else
            {
                // No pending phase, stay idle.
                _phase.reset();
                utils::debug_log(kClassName + L" has no pending phase, entering idle state.");
            }
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
        _phaseId = static_cast<BackdoorMenuPhaseId>(phase);
        // Load other necessary data
        // e.g., background stars, current selection, etc.
        _starField.Load(in);    // Load background stars state
    }

    void BackdoorMenu::SetNextScene(SceneID scene, const Parameters& params)
    {
        _nextScene = scene;
        _nextParams = params;
    }

    void BackdoorMenu::onEnter_(const Parameters& params)
    {
        utils::debug_log(kClassName + L" initialized.");

        using namespace apps::runtime;
        auto& resource = GameContext::GetInstance().GetResourceManager();
        auto& font = resource.GetFontTileManager();
        font.SetUp();

        auto& audio = resource.GetAudioManager();
        audio.Initialize(MM2H_PROPERTY(BackdoorMenuSoundProperty));

        foundation::NES::NESPalette::SetBackgroundFor(13U); // Innocent black

        _phase = std::make_unique<BackdoorMenu_::CreditPhase>(*this);
        _phaseId = _phase->Id();

        PhaseFadePlan first(
            20, // preBlackHold
            20, // fadeInFrames
            0,  // preFadeOutHold
            12, // fadeOutFrames
            20, // postBlackHold
            FadeLayerMask::BG | FadeLayerMask::Font // layers
        );
        _fader.BeginPhase(first, resource);

        _cursor.Load(L"Cursor", MM2H_GRAPHICS(FlatCursor), MM2H_GRAPHPROPS(FlatCursor));
        _cursor.SetBaseTileDuration(40); // Slower twinkle
        vfx::cursor::TwinkleCursorAnimator::Step customLoop[] = {
            {0, 40}, {1, 6}, {2, 6}, {3, 6}, {2, 6}, {1, 6}, {0, 15}
        };
        _cursor.SetLoop(customLoop, std::size(customLoop));

        _starField.InitStars();
        _resource = GameContext::GetInstance().GetResourceManagerPtr();
        _input = &GameContext::GetInstance().Input();
    }

    void BackdoorMenu::onExit_()
    {
        using namespace apps::runtime;
        GameContext::GetInstance().GetResourceManager().GetFontTileManager().ShutDown();
        _phase.reset();
        auto& audio = GameContext::GetInstance().GetResourceManager().GetAudioManager();
        audio.Release();

        utils::debug_log(kClassName + L" finalized.");
    }
}