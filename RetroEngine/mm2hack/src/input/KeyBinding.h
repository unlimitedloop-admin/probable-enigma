//==============================================================================
// 
//  Project: mm2hack
//  KeyBinding.h
// 
//  Key binding management for serial-Controller.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include "JpBtn.h"
#include "KeyToken.h"

namespace mm2hack::input
{
    // Serial-Controller button mapping
    class KeyBinding final
    {
    public:
        KeyBinding() = default;
        ~KeyBinding() = default;
        KeyBinding(const KeyBinding&) = delete;
        KeyBinding& operator=(const KeyBinding&) = delete;
        KeyBinding(KeyBinding&&) = delete;
        KeyBinding& operator=(KeyBinding&&) = delete;
        // KeyBinding is not copyable or movable, so we delete the copy and move constructors and assignment operators.

        bool SetBinding(JPBTN button, uint16_t token);
        bool UnsetBinding(JPBTN button);
        // Set the button mapping for the Serial-Controller
        bool SetBindingSCon(const std::array<uint16_t, JPBTN_COUNT>& buttonMap, bool xInput, bool hatswc, bool trgg, bool thumb);
        // Get the button mapping for the Serial-Controller
        uint16_t GetBindingSCon(JPBTN button) const;
        const std::array<uint16_t, JPBTN_COUNT>& GetAllBindingsSCon() const noexcept;
        // Check if the Serial-Controller for XInput is enabled
        bool IsXInputEnabled() const;
        // Set the Serial-Controller default key binding
        bool SetDefaultBindingSCon(Device type);

    private:
        // Binding array for Serial-Controller
        struct XInputModulation
        {
            std::array<uint16_t, JPBTN_COUNT> button_map{};
            bool xinput_enabled = false;
            bool hatswitch_enabled = false;
            bool trigger_enabled = false;
            bool thumb_enabled = false;

            XInputModulation() = default;
            ~XInputModulation() = default;

            XInputModulation(const XInputModulation&) = delete;
            XInputModulation& operator=(const XInputModulation&) = delete;
            XInputModulation(XInputModulation&&) = delete;
            XInputModulation& operator=(XInputModulation&&) = delete;
        };

        XInputModulation _sCon;     // Serial-Controller binding data
    };
}