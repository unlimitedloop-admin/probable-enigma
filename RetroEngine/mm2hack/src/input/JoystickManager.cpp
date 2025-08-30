#include "pch.h"

#include "JoystickManager.h"

#include <cstdint>
#include <optional>
#include "C16ButtonState.h"
#include "DirectInputProvider.h"
#include "IInputProvider.h"
#include "InputFrame.h"
#include "KeyBinding.h"
#include "KeyboardInputProvider.h"
#include "KeyToken.h"
#include "RawInputEvent.h"
#include "XInputProvider.h"

namespace mm2hack::input
{
    JoystickManager::JoystickManager()
    {
        _binding.SetDefaultBindingSCon();

        if (CheckXInputAvailable())
        {
            _provider = std::make_unique<XInputProvider>(_binding);
        }
        else if (CheckDirectInputAvailable())
        {
            _provider = std::make_unique<DirectInputProvider>(_binding);
        }
        else
        {
            _provider = std::make_unique<KeyboardInputProvider>(_binding);
        }
    }

    bool JoystickManager::Update()
    {
        return _provider->Update(_button_state);
    }

    const InputFrame& JoystickManager::GetButtonState(size_t index) const
    {
        return _button_state.GetState(index);
    }

    const C16ButtonState& JoystickManager::GetAllStates() const
    {
        return _button_state;
    }

    bool JoystickManager::IsEnableInputDevice() const
    {
        return _provider != nullptr;
    }

    std::optional<RawInputEvent> JoystickManager::PollFirstRawChange(float deadzone) noexcept
    {
        // 1) Keyboard（最初に押されたVKを拾う）
        for (int vk = 0; vk < 256; ++vk)
        {
            if (CheckHitKey(vk))
            {
                return RawInputEvent{ RawDevice::Keyboard, RawKind::Key,
                                      static_cast<uint8_t>(vk), false, 1.0f };
            }
        }

        // 2) XInput（Buttons -> Triggers -> Axes の順に見る）
        {
            XINPUT_STATE st{};
            if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &st) == 0)
            {
                // Buttons[]: 0..15（プロジェクトの定義に合わせる）
                for (uint8_t bi = 0; bi < 16; ++bi)
                {
                    if (st.Buttons[bi])
                    {
                        return RawInputEvent{ RawDevice::XInput, RawKind::Button, bi, false, 1.0f };
                    }
                }
                // Triggers
                if (st.LeftTrigger / 255.0f >= deadzone)
                    return RawInputEvent{ RawDevice::XInput, RawKind::Trigger, XI_LT, false, st.LeftTrigger / 255.0f };
                if (st.RightTrigger / 255.0f >= deadzone)
                    return RawInputEvent{ RawDevice::XInput, RawKind::Trigger, XI_RT, false, st.RightTrigger / 255.0f };

                // Axes（LX, LY, RX, RY）
                struct AxisView { int v; uint8_t idx; };
                const AxisView axes[] = {
                    { st.ThumbLX, XI_LX }, { st.ThumbLY, XI_LY },
                    { st.ThumbRX, XI_RX }, { st.ThumbRY, XI_RY }
                };
                const int thr = static_cast<int>(deadzone * 32767.0f + 0.5f);
                for (auto a : axes)
                {
                    if (a.v >= thr)  return RawInputEvent{ RawDevice::XInput, RawKind::Axis, a.idx, false,  static_cast<float>(a.v) / 32767.0f };
                    if (a.v <= -thr) return RawInputEvent{ RawDevice::XInput, RawKind::Axis, a.idx, true,  static_cast<float>(-a.v) / 32767.0f };
                }
            }
        }

        // 3) DirectInput（必要になったところで追加）
        //  ...（省略）

        return std::nullopt;
    }

    bool JoystickManager::CheckXInputAvailable()
    {
        XINPUT_STATE state{};
        if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }
        return true;
    }

    bool JoystickManager::CheckDirectInputAvailable()
    {
        DINPUT_JOYSTATE state{};
        if (GetJoypadDirectInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }

        if (state.POV[0] >= 0)
        {
            return true;
        }

        for (int i = 0; i < 32; ++i)
        {
            if (state.Buttons[i] != 0)
            {
                return true;
            }
        }

        return false;
    }
}