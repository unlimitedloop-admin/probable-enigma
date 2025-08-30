#include "pch.h"

#include "InputConfigOverlay.h"

#include <cstdint>
#include "apps/deal/GameContext.h"
#include "core/GameState.h"
#include "core/GameStateManager.h"
#include "input/Jpbtn.h"
#include "input/KeyBinding.h"
#include "input/KeyToken.h"
#include "input/RawInputEvent.h"
#include "utils/string_converter.h"

namespace mm2hack::core::overlay
{
    namespace
    {
        [[nodiscard]] uint16_t ToXInputToken(const input::RawInputEvent& e) noexcept
        {
            // 必要ならここで Trigger/Axis も拡張（0x1xxx など）
            return static_cast<uint16_t>(e.code); // ボタンだけならこれで十分
        }

        // CaptureStep が JPBTN を持っている前提に寄せる
        [[nodiscard]] JPBTN StepToJpbtn(const CaptureStep& s) noexcept
        {
            return s.jpbtn;
        }
    }


    using input::MakeToken; using input::Device; using input::XI_LT; using input::XI_RT;

    static uint16_t ToToken(const input::RawInputEvent& e, uint8_t thrNibbleDefault)
    {
        using RD = input::RawDevice; using RK = input::RawKind;
        switch (e.device)
        {
        case RD::Keyboard:
            return MakeToken(Device::Keyboard, false, e.code, false, 0);
        case RD::XInput:
            if (e.kind == RK::Button)  return MakeToken(Device::XInput, false, e.code);
            if (e.kind == RK::Trigger) return MakeToken(Device::XInput, true, e.code, false, thrNibbleDefault);
            if (e.kind == RK::Axis)    return MakeToken(Device::XInput, true, e.code, e.negative, thrNibbleDefault);
            break;
        case RD::DirectInput:
            return MakeToken(Device::DirectInput, e.kind != RK::Button, e.code, e.negative, thrNibbleDefault);
        }
        return 0xFFFFui16;
    }

    void InputConfigOverlay::Open(input::KeyBinding& target, std::vector<CaptureStep> steps)
    {
        _target = &target;
        _steps = std::move(steps);
        _index = 0;

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

    void InputConfigOverlay::Cancel() noexcept
    {
        _state = CaptureState::Cancelled;
        GameStateManager::GetInstance().SetState(GameState::Paused);
    }

    bool InputConfigOverlay::IsOpen() const noexcept
    {
        return _state != CaptureState::Hidden;
    }

    void InputConfigOverlay::Tick(float dt)
    {
        if (_state == CaptureState::Hidden) return;

        if (/* _state == CaptureState::Completed || */_state == CaptureState::Cancelled)
        {
            _state = CaptureState::Hidden;
            GameStateManager::GetInstance().SetState(GameState::Paused);
            return;
        }

        if (CheckHitKey(KEY_INPUT_ESCAPE))
        {
            Cancel(); return;
        }
        if (_state == CaptureState::Listening && CheckHitKey(KEY_INPUT_BACK))
        {
            _target->UnsetBinding(_steps[_index].jpbtn); advance(); return;
        }
        if (_state == CaptureState::Listening && CheckHitKey(KEY_INPUT_RETURN))
        {
            advance(); return;
        }

        using enum CaptureState;
        auto& jm = apps::deal::GameContext::GetInstance().GetJoystickManager();
        switch (_state)
        {
        case Intro:
            if (std::chrono::steady_clock::now() - _stateStart > std::chrono::milliseconds(200))
                _state = Quiescent, _stateStart = std::chrono::steady_clock::now();
            break;
        case Quiescent:
            // 150ms 入力が静かなら Listening へ
            if (std::chrono::steady_clock::now() - _stateStart > std::chrono::milliseconds(_requiredQuiescentMs))
                _state = Listening;
            // 何か入力があれば静穏やり直し
            if (jm.PollFirstRawChange(_analogThreshold))
                _stateStart = std::chrono::steady_clock::now();
            break;
        case Listening:
            if (auto evt = jm.PollFirstRawChange(_analogThreshold))
            {
                adoptBinding(*evt);    // KeyBinding に反映（重複ポリシーもここで適用）
                advance();
            }
            break;
        case Confirm: /* シンプル版は即 advance */ break;
        case Cancelled:
            _state = Hidden;
            break;
        default:
            break;
        }

        render();
    }

    void InputConfigOverlay::advance()
    {
        if (++_index >= _steps.size())
        {
            _state = CaptureState::Completed;
            return;
        }
        _state = CaptureState::Quiescent;
        _stateStart = std::chrono::steady_clock::now();
    }

    void InputConfigOverlay::adoptBinding(const input::RawInputEvent& e)
    {
        const auto jp = StepToJpbtn(_steps[_index]);
        const uint16_t token = ToXInputToken(e);
        _target->SetBinding(jp, token);
    }

    void InputConfigOverlay::render() const
    {
        using conf = config::SystemConfig;
        // Alpha blending.
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
        DrawBox(0, 0, conf::kScreenWidth, conf::kScreenHeight, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

        if (_index >= _steps.size())
        {
            DrawString(40, 80, L"Configuration completed.\nPress [Esc] to exit.", GetColor(255, 255, 255));
            return;
        }

        // Draw instructions.
        const auto& step = _steps[_index];
        std::wstring msg = L"Press the \"" + utils::utf8_to_wstring(step.label) + L"\" key on your device.\n"
            L"[Esc] Cancel  [Enter] Skip  [Backspace] Unbind";
        DrawString(40, 80, msg.c_str(), GetColor(255, 255, 255));

        // 右上に現在のバインド一覧と採用ルール、左下に接続デバイス名など（省略）
    }
}