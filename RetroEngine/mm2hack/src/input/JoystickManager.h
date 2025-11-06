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
#include <string>
#include "C16ButtonState.h"
#include "IInputProvider.h"
#include "InputFrame.h"
#include "Jpbtn.h"
#include "KeyBinding.h"
#include "KeyToken.h"
#include "RawInputEvent.h"

namespace mm2hack::input
{
    enum class AxisGroup : uint8_t { Any, Left, Right };

    // Manages key input devices, specifically joystick input (Keyboards and Gamepads)
    class JoystickManager final
    {
    public:
        explicit JoystickManager();

        // Update the joystick state and return true if the state has changed
        bool Update();
        // Get the state of a specific button by its index
        const InputFrame& GetButtonState(size_t index) const;
        // Get the state of a specific button by its JPBTN enum
        const InputFrame& GetButtonState(JPBTN button) const { return GetButtonState(static_cast<size_t>(button)); }
        // Get the state of all buttons
        const C16ButtonState& GetAllStates() const;
        // Check if the input device is enabled
        bool IsEnableInputDevice() const;

        // Set or unset the binding for a specific button
        bool ApplyBindingOne(JPBTN button, uint16_t token) { return _binding.SetBinding(button, token); }
        bool UnsetBindingOne(JPBTN button) { return _binding.UnsetBinding(button); }

        // Poll the first raw input change event, with optional deadzone and device filter
        std::optional<RawInputEvent> PollFirstRawChange(float deadzone = 0.5f, std::optional<Device> only = std::nullopt) const noexcept;

        // Set the DirectInput axis capture group (Any, Left, Right)
        void SetDirectInputCaptureGroup(AxisGroup g) noexcept { _diCaptureGroup = g; }

        // Getters for other properties
        KeyBinding& GetKeyBinding() noexcept { return _binding; }
        const KeyBinding& GetKeyBinding() const noexcept { return _binding; }
        Device ActiveDevice() const noexcept { return _activeKind; }

    private:
        const std::wstring kClassName = L"JoystickManager";

        std::unique_ptr<IInputProvider> _provider;      // Pointer to the input provider
        C16ButtonState _button_state;                   // Current state of all buttons
        KeyBinding _binding;                            // Key binding configuration

        AxisGroup _diCaptureGroup{ AxisGroup::Left };   // DirectInput axis capture group
        Device _activeKind{ Device::Keyboard };         // Currently active input device kind

        bool checkXInputAvailable_();                   // Check if XInput is available
        bool checkDirectInputAvailable_();              // Check if DirectInput is available
    };
}