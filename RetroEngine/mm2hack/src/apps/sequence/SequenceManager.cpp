#include "pch.h"

#include "SequenceManager.h"

#include "apps/deal/GameContext.h"
#include "core/GameState.h"
#include "core/GameStateManager.h"
#include "core/overlay/DebugHud.h"
#include "core/overlay/InputConfigOverlay.h"
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
        auto& gsm = core::GameStateManager::GetInstance();
        if (_currentSequence)
        {
            _feedbackOverlay.Update();
            if (gsm.IsRunning()) _currentSequence->Execute();
        }
    }

    void SequenceManager::RenderWorld(int screenHandle, float viewerRate)
    {
        using conf = config::SystemConfig;
        if (_currentSequence)
        {
            _currentSequence->RenderWorld();
        }

        // Scale what we draw to fit the viewer rate.
        DxLib::SetDrawScreen(DX_SCREEN_BACK);
        DxLib::DrawExtendGraph(0, 0,
                static_cast<int>(conf::kScreenWidth * viewerRate),
                static_cast<int>(conf::kScreenHeight * viewerRate),
                screenHandle, FALSE
        );
    }

    void SequenceManager::RenderOverlay(float viewerRate)
    {
        using conf = config::SystemConfig;
        if (_currentSequence)
        {
            _currentSequence->RenderOverlay();
        }
        _feedbackOverlay.Render(
            static_cast<int>(conf::kScreenWidth * viewerRate),
            static_cast<int>(conf::kScreenHeight * viewerRate)
        );

        core::overlay::DebugHud::GetInstance().Draw();     // Draw the FPS in the HUD, top-left corner
    }

    void SequenceManager::Release()
    {
        if (_currentSequence)
        {
            _currentSequence.reset();
            _sequenceType = SequenceType::None;
        }
    }

    void SequenceManager::SendFeedback(const std::wstring& message)
    {
        _feedbackOverlay.ShowMessage(message, 180);
    }

    void SequenceManager::HandleJpbtnConfigMode(double dt)
    {
        using namespace apps::deal;
        using namespace core;
        // If we are in JPBTN configuration mode, update the joystick manager and tick the input config overlay.
        if (GameStateManager::GetInstance().Is(GameState::JpbtnConfig))
        {
            GameContext::GetInstance().GetJoystickManager().Update();
            overlay::InputConfigOverlay::GetInstance().Tick(static_cast<float>(dt));
        }
    }
}