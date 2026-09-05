#include "pch.h"

#include "BackdoorMenu.h"

#include <cstdint>
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

    bool BackdoorMenu::CanSaveState() const noexcept
    {
        return _phase != nullptr && _pendingPhase == nullptr && !_leaving &&
            _nextScene == SceneID::None &&
            _fader.Current() == PhaseFadeController::State::Interactive;
    }

    bool BackdoorMenu::Save(std::ostream& out) const
    {
        constexpr std::uint32_t kStateVersion = 1;
        if (!CanSaveState()) return false;

        core::save::StateWriter writer(out);
        if (!writer.WriteU32(kStateVersion) ||
            !writer.WriteI32(static_cast<std::int32_t>(_phase->Id())) ||
            !_phase->Save(writer) ||
            !writer.WriteU32(static_cast<std::uint32_t>(_cursor.AnimationStepIndex())) ||
            !writer.WriteI32(_cursor.AnimationTicks()))
        {
            return false;
        }
        return _starField.Save(out);
    }

    bool BackdoorMenu::Load(std::istream& in)
    {
        constexpr std::uint32_t kStateVersion = 1;
        core::save::StateReader reader(in);
        std::uint32_t state_version{};
        std::int32_t phase_value{};
        if (!reader.ReadU32(state_version) || state_version != kStateVersion ||
            !reader.ReadI32(phase_value))
        {
            return false;
        }

        const auto phase_id = static_cast<BackdoorMenuPhaseId>(phase_value);
        std::unique_ptr<IBackdoorMenuPhase> phase;
        switch (phase_id)
        {
        case BackdoorMenuPhaseId::Credit:
            phase = std::make_unique<BackdoorMenu_::CreditPhase>(*this);
            break;
        case BackdoorMenuPhaseId::TopMenu:
            phase = std::make_unique<BackdoorMenu_::TopMenuPhase>(*this);
            break;
        case BackdoorMenuPhaseId::InsideMenu:
            phase = std::make_unique<BackdoorMenu_::InsideMenuPhase>(*this);
            break;
        default:
            return false;
        }

        if (!phase->Load(reader)) return false;

        std::uint32_t cursor_step{};
        std::int32_t cursor_ticks{};
        if (!reader.ReadU32(cursor_step) || !reader.ReadI32(cursor_ticks) ||
            !_cursor.CanRestoreAnimation(cursor_step, cursor_ticks) || _resource == nullptr)
        {
            return false;
        }

        // BgStarField validates into temporary containers before committing.
        // No fallible operation follows it, keeping this scene load transactional.
        if (!_starField.Load(in)) return false;

        _phase = std::move(phase);
        _phaseId = phase_id;
        _pendingPhase.reset();
        _pendingPlan = {};
        _nextScene = SceneID::None;
        _nextParams = {};
        _leaving = false;
        _cursor.RestoreAnimation(cursor_step, cursor_ticks);
        _fader.RestoreInteractive(*_resource);
        return true;
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
        auto& font = GameContext::GetInstance().GetResourceManager().GetFontTileManager();
        font.ShutDown();

        utils::debug_log(kClassName + L" finalized.");
    }
}
