//==============================================================================
// 
//  Project: mm2hack
//  JoystickManager.h
// 
//  JoystickManager is a class that manages the joystick input.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include "C16ButtonState.h"
#include "IInputProvider.h"
#include "InputFrame.h"
#include "Jpbtn.h"
#include "KeyBinding.h"
#include "RawInputEvent.h"

namespace mm2hack::input
{
    // Manages key input devices, specifically joystick input (Keyboards and Gamepads)
    class JoystickManager final
    {
    public:
        explicit JoystickManager();

        // Update the joystick state and return true if the state has changed
        bool Update();
        // Get the state of a specific button by its index
        const InputFrame& GetButtonState(size_t index) const;
        const InputFrame& GetButtonState(JPBTN button) const
        {
            return GetButtonState(static_cast<size_t>(button));
        }
        // Get the state of all buttons
        const C16ButtonState& GetAllStates() const;
        // Check if the input device is enabled
        bool IsEnableInputDevice() const;

        bool ApplyBindingOne(JPBTN button, uint16_t token) { return _binding.SetBinding(button, token); }
        bool UnsetBindingOne(JPBTN button) { return _binding.UnsetBinding(button); }

        std::optional<RawInputEvent> PollFirstRawChange(float deadzone = 0.5f) noexcept;

        KeyBinding& GetKeyBinding() noexcept { return _binding; }

    private:
        std::unique_ptr<IInputProvider> _provider;      // Pointer to the input provider
        C16ButtonState _button_state;
        KeyBinding _binding;

        // Check if XInput is available
        bool CheckXInputAvailable();
        // Check if DirectInput is available
        bool CheckDirectInputAvailable();
    };
}