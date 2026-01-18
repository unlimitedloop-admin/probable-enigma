#include "pch.h"

#include "InputConfigOverlay.h"

#include "apps/runtime/GameContext.h"
#include "config/ConfigUIManager.h"
#include "core/GameState.h"
#include "core/GameStateManager.h"
#include "core/winapi/WindowManager.h"
#include "input/di/DirectInputToken.h"
#include "input/JoystickManager.h"
#include "input/Jpbtn.h"
#include "input/KeyBinding.h"
#include "input/KeyToken.h"
#include "input/RawInputEvent.h"
#include "input/xi/XInputToken.h"
#include "utils/string_converter.h"

namespace mm2hack::core::overlay
{
    namespace
    {
        [[nodiscard]] uint16_t ToXInputToken(const input::RawInputEvent& e) noexcept
        {
            using namespace input;
            using namespace xi;
            using RK = RawKind;
            constexpr uint8_t kDefaultThr = 8;  // nearly 50% threshold

            if (e.kind == RK::Button)  return MakeBtn(e.code);
            if (e.kind == RK::Trigger) return MakeTrig(e.code, kDefaultThr);
            if (e.kind == RK::Axis)    return MakeAxis(e.code, e.negative, kDefaultThr);
            return kTokenUnbound;
        }

        [[nodiscard]] uint16_t ToDirectInputToken(const input::RawInputEvent& e) noexcept
        {
            using namespace input;
            using namespace di;
            using RK = RawKind;
            constexpr uint8_t kThr = 8;

            if (e.kind == RK::Button) return MakeBtn(e.code);
            if (e.kind == RK::Axis)   return MakeAxis(e.code, e.negative, kThr);
            if (e.kind == RK::POV)    return MakePOV(e.code);
            return kTokenUnbound;
        }
    }

    void InputConfigOverlay::Open(input::KeyBinding& target, std::vector<CaptureStep> steps)
    {
        _target = &target;
        _steps = std::move(steps);
        _index = 0;
        _captureKind = apps::runtime::GameContext::GetInstance().Joystick().ActiveDevice();

        if (_steps.empty())
        {
            _state = CaptureState::Completed;   // Exit immediately if no steps.
        }
        else
        {
            _state = CaptureState::Intro;
            _stateStart = std::chrono::steady_clock::now();
        }
        GameStateManager::GetInstance().SetState(GameState::JpbtnConfig);
    }

    bool InputConfigOverlay::IsOpen() const noexcept
    {
        return _state != CaptureState::Hidden;
    }

    void InputConfigOverlay::Tick(float dt)
    {
        if (_state == CaptureState::Hidden) return;

        if (_state == CaptureState::Cancelled)
        {
            Finish(false);
            return;
        }

        if (CheckHitKey(KEY_INPUT_ESCAPE))
        {
            _state = CaptureState::Cancelled;
            return;
        }
        if (_state == CaptureState::Listening && CheckHitKey(KEY_INPUT_BACK))
        {
            _target->UnsetBinding(_steps[_index].jpbtn);
            advance_();
            return;
        }
        if (_state == CaptureState::Listening && CheckHitKey(KEY_INPUT_RETURN))
        {
            advance_();
            return;
        }

        using enum CaptureState;
        auto& jm = apps::runtime::GameContext::GetInstance().Joystick();
        switch (_state)
        {
        case Intro:
            if (std::chrono::steady_clock::now() - _stateStart > std::chrono::milliseconds(200))
                _state = Quiescent, _stateStart = std::chrono::steady_clock::now();
            break;
        case Quiescent:
            // If quiescent time has passed, go to Listening state.
            if (std::chrono::steady_clock::now() - _stateStart > std::chrono::milliseconds(_requiredQuiescentMs))
                _state = Listening;
            // If there is any input, restart quiescent period.
            if (jm.PollFirstRawChange(_analogThreshold))
                _stateStart = std::chrono::steady_clock::now();
            break;
        case Listening:
            if (auto evt = jm.PollFirstRawChange(_analogThreshold, _captureKind))
            {
                adoptBinding_(*evt);    // Assign to KeyBinding.
                _state = Confirm;
                _confirmElapsedSec = 0.0f;
                _confirmQuietSec = 0.0f;
            }
            break;
        case Confirm:
        {
            _confirmElapsedSec += dt;
            _confirmQuietSec = jm.PollFirstRawChange(_analogThreshold, _captureKind) ? 0.0f : _confirmQuietSec + dt;
            const bool shownEnough = (_confirmElapsedSec * 1000.0f) >= _confirmMinShowMs;
            const bool quietEnough = (_confirmQuietSec * 1000.0f) >= _confirmQuietMs;

            if (shownEnough && quietEnough)
            {
                advance_();  // Go to next step.
            }
            break;
        }
        case Completed:
            if (CheckHitKey(KEY_INPUT_RETURN))
            {
                Finish(true);  // Finish and save.
                return;
            }
            break;
        default:
            break;
        }

        render_();
    }

    void InputConfigOverlay::Finish(bool committed)
    {
        // Whether to save or not, enable saveOnCancel if you want to save when canceled.
        if (committed /*|| saveOnCancel*/)
        {
            auto& jm = apps::runtime::GameContext::GetInstance().Joystick();
            config::ConfigUIManager::SaveInputDeviceConfig(jm.GetKeyBinding(), jm.ActiveDevice());
        }

        _state = CaptureState::Hidden;
        core::GameStateManager::GetInstance().SetState(core::GameState::Running);
    }

    void InputConfigOverlay::render_() const
    {
        using conf = config::SystemConfig;
        auto viewerRate = core::winapi::WindowManager::GetInstance().GetViewerRate();
        // Alpha blending.
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
        DrawBox(0, 0,
            static_cast<int>(conf::kScreenWidth * viewerRate),
            static_cast<int>(conf::kScreenHeight * viewerRate),
            GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        if (_index >= _steps.size())
        {
            DrawString(40, 80, L"Configuration completed.\nSave settings to an ini file? Press [Enter] key.\nOr if not, press [Esc] to exit.", GetColor(255, 255, 255));
            return;
        }

        if (_state == CaptureState::Confirm)
        {
            DrawString(40, 80, L"Binding adopted.\nPlease wait...", GetColor(192, 192, 192));
            return;
        }

        // Draw instructions.
        const auto& step = _steps[_index];
        std::wstring msg = L"Press the \"" + utils::utf8_to_wstring(step.label) + L"\" key on your device.\n"
            L"[Esc] Cancel  [Enter] Skip  [Backspace] Unbind";
        DrawString(40, 80, msg.c_str(), GetColor(255, 255, 255));

        // Draw current binding list and adoption rules in the top right, device name in the bottom left (omitted).
    }

    void InputConfigOverlay::advance_()
    {
        if (++_index >= _steps.size())
        {
            _state = CaptureState::Completed;
            return;
        }
        onStepEntered_();
        _state = CaptureState::Quiescent;
        _stateStart = std::chrono::steady_clock::now();
    }

    void InputConfigOverlay::adoptBinding_(const input::RawInputEvent& e)
    {
        using namespace input;

        if (_index >= _steps.size()) return;
        const auto jp = _steps[_index].jpbtn;

        uint16_t token = kTokenUnbound;
        switch (_captureKind)
        {
        case Device::XInput:      token = ToXInputToken(e);              break;
        case Device::DirectInput: token = ToDirectInputToken(e);         break;
        case Device::Keyboard:    token = static_cast<uint16_t>(e.code); break; // VK
        }
        if (IsUnboundToken(token)) { return; }

        // Assigning to single button (1:1 mapping assumed).
        _target->SetBinding(jp, token);

        // NOTE: If you assign the same token to multiple buttons, you need to add logic here to unbind the existing assignment.
    }

    void InputConfigOverlay::onStepEntered_()
    {
        using namespace input;

        if (_index >= _steps.size()) return;
        if (_captureKind == Device::DirectInput)
        {
            const auto jp = _steps[_index].jpbtn;
            auto& JM = apps::runtime::GameContext::GetInstance().Joystick();
            switch (jp)
            {
            case JPBTN::UP:
            case JPBTN::DOWN:
            case JPBTN::LEFT:
            case JPBTN::RIGHT:
                JM.SetDirectInputCaptureGroup(AxisGroup::Left);
                break;
            default:
                // If you need to capture Right stick axes, change here. (use AxisGroup::Right)
                JM.SetDirectInputCaptureGroup(AxisGroup::Any);
                break;
            }
        }
    }
}