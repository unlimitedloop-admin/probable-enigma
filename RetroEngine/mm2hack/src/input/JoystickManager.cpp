#include "pch.h"

#include "JoystickManager.h"

#include "C16ButtonState.h"
#include "DirectInputProvider.h"
#include "IInputProvider.h"
#include "InputFrame.h"
#include "KeyBinding.h"
#include "KeyboardInputProvider.h"
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