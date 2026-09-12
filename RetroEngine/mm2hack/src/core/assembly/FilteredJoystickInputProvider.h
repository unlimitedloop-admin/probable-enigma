//==============================================================================
// 
//  Project: mm2hack
//  FilteredJoystickInputProvider.h
// 
//  Filters joystick input based on enabled state.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "input/JoystickManager.h"
#include "InputTypes.h"
#include "JoystickInputProviderAdapter.h"

namespace mm2hack::core::assembly
{
    // Gate-enabled joystick provider (derived from JoystickInputProviderAdapter).
    class FilteredJoystickInputProvider final : public JoystickInputProviderAdapter
    {
    public:
        explicit FilteredJoystickInputProvider(input::JoystickManager& jm) noexcept
            : JoystickInputProviderAdapter(jm)
        {
        }

        // Enable or disable input filtering
        void SetEnabled(bool enabled) noexcept
        {
            _enabled = enabled;
        }

        // Check if input filtering is enabled
        [[nodiscard]] bool Enabled() const noexcept
        {
            return _enabled;
        }

        // ---- Single item query (gated) ----
        // Get key state if enabled, otherwise zero state
        [[nodiscard]] const KeyFrameState& Get(Key16 k) const noexcept override
        {
            return _enabled ? JoystickInputProviderAdapter::Get(k) : _zeroState_;
        }

        // True if key is pressed and input enabled
        [[nodiscard]] bool IsPressed(Key16 k) const noexcept override
        {
            return _enabled ? JoystickInputProviderAdapter::IsPressed(k) : false;
        }

        // True if key was just pressed and input enabled
        [[nodiscard]] bool JustPressed(Key16 k) const noexcept override
        {
            return _enabled ? JoystickInputProviderAdapter::JustPressed(k) : false;
        }

        // True if key was just released and input enabled
        [[nodiscard]] bool JustReleased(Key16 k) const noexcept override
        {
            return _enabled ? JoystickInputProviderAdapter::JustReleased(k) : false;
        }

        // Frame count for key if enabled, else 0
        [[nodiscard]] std::int32_t Frames(Key16 k) const noexcept override
        {
            return _enabled ? JoystickInputProviderAdapter::Frames(k) : 0;
        }

    private:
        bool _enabled{ true };
        inline static const KeyFrameState _zeroState_{ false, 0, false };
    };
}