//==============================================================================
// 
//  Project: mm2hack
//  IInputBackend.h
// 
//  Input backend interface for handling user input.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "input/KeyToken.h"

namespace mm2hack::core::assembly
{
    // Abstract ID for physical inputs (Keyboard holds virtual keys/scan codes, XInput holds button bits and axis codes)
    struct PhysicalBinding
    {
        input::Device device{};
        std::uint16_t code{};   // Ex: VK_*, XINPUT_GAMEPAD_*
    };

    // Interface for low-level input backend (keyboard, XInput, DirectInput, etc.)
    class IInputBackend
    {
    public:
        virtual ~IInputBackend() = default;

        // Called at the start of each frame (update hardware state)
        virtual void UpdateHardware() noexcept = 0;

        // Digital value (button/direction). True if pressed
        [[nodiscard]] virtual bool GetDigital(input::Device dev, std::uint16_t code) const noexcept = 0;

        // Analog value (axis/trigger). Policy is to return -1..1 or 0..1 (threshold is determined at a higher level)
        [[nodiscard]] virtual float GetAnalog(input::Device dev, std::uint16_t code) const noexcept = 0;
    };
}
