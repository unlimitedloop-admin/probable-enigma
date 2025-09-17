//==============================================================================
// 
//  Project: mm2hack
//  RawInputEvent.h
// 
//  Raw input event structure.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::input
{
    enum class RawDevice : uint8_t { Keyboard, XInput, DirectInput };
    enum class RawKind : uint8_t { Button, Trigger, Axis, Key, POV };

    // Represents a low-level input event from keyboard or joystick
    struct RawInputEvent
    {
        RawDevice device;
        RawKind   kind;
        uint8_t   code;     // Button idx / Axis idx / Trigger idx / VK
        bool      negative; // Use only for Axis
        float     value;    // Normalized value (0.0 to 1.0)
    };
}