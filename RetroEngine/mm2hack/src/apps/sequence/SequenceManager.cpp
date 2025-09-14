#include "pch.h"

#include "SequenceManager.h"

#include "apps/deal/GameContext.h"
#include "core/GameState.h"
#include "core/GameStateManager.h"
#include "core/overlay/DebugHud.h"
#include "core/overlay/InputConfigOverlay.h"
#include "core/overlay/PauseManager.h"
#include "DebugSequence.h"
#include "SequenceType.h"
#include "StandardSequence.h"
#include "test/TestSequence.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::sequence
{
    void SequenceManager::StartStandardSequence()
    {
        utils::debug_log(L"Start standard sequence.");
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<StandardSequence>();
        _sequenceType = SequenceType::Standard;
    }

    void SequenceManager::StartDebugSequence()
    {
        utils::debug_log(L"Start debug sequence.");
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<DebugSequence>();
        _sequenceType = SequenceType::Debug;
    }

    void SequenceManager::StartTestSequence(const int no)
    {
        utils::debug_log(L"Start test sequence.");
        if (_currentSequence)
        {
            StopCurrentSequence();
        }
        _currentSequence = std::make_unique<TestSequence>(no);
    }

    void SequenceManager::StopCurrentSequence()
    {
        utils::debug_log(L"Stop current sequence.");
        Release();
    }

    void SequenceManager::RebootCurrentSequence()
    {
        utils::debug_log(L"Reboot sequence.");

        switch (_sequenceType)
        {
        case SequenceType::Standard:
            StartStandardSequence();
            break;
        case SequenceType::Debug:
            StartDebugSequence();
            break;
        default:
            utils::debug_log(L"No sequence to reboot.");
            break;
        }
    }

    void SequenceManager::LoadSequence(const SequenceType type)
    {
        utils::debug_log(L"Load sequence from sav file.");

        switch (type)
        {
        case SequenceType::Standard:
            StartStandardSequence();
            break;
        case SequenceType::Debug:
            StartDebugSequence();
            break;
        default:
            break;
        }
    }

    void SequenceManager::Update()
    {
        using namespace core;
        using namespace deal;

        if (_currentSequence)
        {
            _feedbackOverlay.Update();

            auto& time = GameContext::GetInstance().Time();
            const bool running = GameStateManager::GetInstance().IsRunning();
            const bool shouldAdvance = running || (time.DeltaSeconds() > 0.0);

            // Is the game running or are we in frame advance mode?
            if (shouldAdvance)
            {
                auto& input = GameContext::GetInstance().Input();
                // Advance the input state for this frame.
                input.BeginTick(time.FrameCounter());
                
                _currentSequence->Execute();    // !Execute the main game logic.
                
                // Finalize the input state for this frame.
                input.EndTick();
                // Increment the play frame counter if the game is running. (use in HUD overlays)
                time.IncrementPlayFrameCounter();
            }
        }
    }

    void SequenceManager::RenderWorld(int screenHandle, int destW, int destH)
    {
        using namespace core::overlay;
        using conf = config::SystemConfig;

        if (_currentSequence)
        {
            _currentSequence->RenderWorld();
        }

        if (PauseManager::IsPaused())
        {
            PauseManager::DrawOverlay();
        }

        // Scale what we draw to fit the viewer rate.
        DxLib::SetDrawScreen(DX_SCREEN_BACK);
        DxLib::DrawExtendGraph(0, 0, destW, destH, screenHandle, FALSE);
    }

    void SequenceManager::RenderOverlay(int destW, int destH)
    {
        using conf = config::SystemConfig;
        if (_currentSequence)
        {
            _currentSequence->RenderOverlay();
        }
        _feedbackOverlay.Render(destW, destH);

        core::overlay::DebugHud::GetInstance().Draw();     // Draw the FPS in the HUD, top-left corner
    }

    void SequenceManager::Release()
    {
        if (_currentSequence)
        {
            _currentSequence.reset();
            _sequenceType = SequenceType::None;

            if (auto* time = deal::GameContext::GetInstance().TryTime())
            {
                time->ResetPlayFrameCounter();
            }
        }
    }

    void SequenceManager::HandleJpbtnConfigMode(double dt)
    {
        using namespace apps::deal;
        using namespace core;
        // If we are in JPBTN configuration mode, update the joystick manager and tick the input config overlay.
        if (GameStateManager::GetInstance().Is(GameState::JpbtnConfig))
        {
            GameContext::GetInstance().Input().UpdateJoystick();
            overlay::InputConfigOverlay::GetInstance().Tick(static_cast<float>(dt));
        }
    }

    void SequenceManager::SendFeedback(const std::wstring& message)
    {
        // Your message will appear as a slide-in overlay for the duration you specify.
        // It's automatic animation.
        _feedbackOverlay.ShowMessage(message, config::SystemConfig::kFeedbackOverlayDuration);
    }
}