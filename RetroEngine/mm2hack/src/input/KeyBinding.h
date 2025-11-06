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
#include <string>
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

        // Set the button mapping for a specific button
        bool SetBinding(JPBTN button, uint16_t token);
        // Set the button mapping to unbound for a specific button
        bool UnsetBinding(JPBTN button);
        // Set the button mapping for the Serial-Controller
        bool SetBindingSCon(const std::array<uint16_t, JPBTN_COUNT>& buttonMap, bool xInput, bool hatswc, bool trgg, bool thumb);
        // Get the button mapping for the Serial-Controller
        uint16_t GetBindingSCon(JPBTN button) const;
        // Get all button mappings for the Serial-Controller
        const std::array<uint16_t, JPBTN_COUNT>& GetAllBindingsSCon() const noexcept;
        
        // Getters for Serial-Controller configuration flags
        bool IsXInputEnabled() const noexcept { return _sCon.xinput_enabled; }
        bool IsHatSwitchEnabled() const noexcept { return _sCon.hatswitch_enabled; }
        bool IsTriggerEnabled() const noexcept { return _sCon.trigger_enabled; }
        bool IsThumbEnabled() const noexcept { return _sCon.thumb_enabled; }

        // Setters for Serial-Controller configuration flags
        void SetFeatureFlags(bool xinput, bool hatswitch, bool trigger, bool thumb) noexcept
        {
            _sCon.xinput_enabled = xinput;
            _sCon.hatswitch_enabled = hatswitch;
            _sCon.trigger_enabled = trigger;
            _sCon.thumb_enabled = thumb;
        }
        
        // Set the Serial-Controller default key binding
        bool SetDefaultBindingSCon(Device type);

    private:
        // Binding array for Serial-Controller
        struct SConBindingData
        {
            std::array<uint16_t, JPBTN_COUNT> button_map{};
            bool xinput_enabled = false;
            bool hatswitch_enabled = false;
            bool trigger_enabled = false;
            bool thumb_enabled = false;

            SConBindingData() = default;
            ~SConBindingData() = default;

            SConBindingData(const SConBindingData&) = delete;
            SConBindingData& operator=(const SConBindingData&) = delete;
            SConBindingData(SConBindingData&&) = delete;
            SConBindingData& operator=(SConBindingData&&) = delete;
        };

        const std::wstring kClassName = L"KeyBinding";
        SConBindingData _sCon;     // Serial-Controller binding data
    };
}