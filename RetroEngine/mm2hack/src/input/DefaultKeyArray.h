//==============================================================================
// 
//  Project: mm2hack
//  DefaultKeyArray.h
// 
//  Keyboard input mapping of the default keys for the gamepad buttons.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include <DxLib.h>
#include "JpBtn.h"

inline constexpr std::array<uint16_t, JPBTN_COUNT> GetDefaultKeyArray()
{
    return {
        KEY_INPUT_W,         // UP
        KEY_INPUT_S,         // DOWN
        KEY_INPUT_A,         // LEFT
        KEY_INPUT_D,         // RIGHT
        KEY_INPUT_M,         // A
        KEY_INPUT_N,         // B
        KEY_INPUT_L,         // X
        KEY_INPUT_K,         // Y
        KEY_INPUT_RETURN,    // START
        KEY_INPUT_SPACE,     // BACK
        KEY_INPUT_1,         // LSHOULDER
        KEY_INPUT_0,         // RSHOULDER
        KEY_INPUT_5,         // LTRIGGER
        KEY_INPUT_7,         // RTRIGGER
        KEY_INPUT_F,         // LTHUMB
        KEY_INPUT_H          // RTHUMB
    };
}

inline constexpr std::array<uint16_t, JPBTN_COUNT> GetDefaultXInputArray()
{
    return {
        XINPUT_BUTTON_DPAD_UP,          // UP
        XINPUT_BUTTON_DPAD_DOWN,        // DOWN
        XINPUT_BUTTON_DPAD_LEFT,        // LEFT
        XINPUT_BUTTON_DPAD_RIGHT,       // RIGHT
        XINPUT_BUTTON_A,                // A
        XINPUT_BUTTON_B,                // B
        XINPUT_BUTTON_X,                // X
        XINPUT_BUTTON_Y,                // Y
        XINPUT_BUTTON_START,            // START
        XINPUT_BUTTON_BACK,             // BACK
        XINPUT_BUTTON_LEFT_SHOULDER,    // LSHOULDER
        XINPUT_BUTTON_RIGHT_SHOULDER,   // RSHOULDER
        static_cast<uint16_t>(-1),      // LTRIGGER
        static_cast<uint16_t>(-1),      // RTRIGGER
        XINPUT_BUTTON_LEFT_THUMB,       // LTHUMB
        XINPUT_BUTTON_RIGHT_THUMB       // RTHUMB
    };
}

inline constexpr std::array<uint16_t, JPBTN_COUNT> GetDefaultDirectInputArray()
{
    return {
        0x0001, // UP (DPad Up)
        0x0002, // DOWN (DPad Down)
        0x0004, // LEFT (DPad Left)
        0x0008, // RIGHT (DPad Right)
        0x1000, // A (Button 1)
        0x2000, // B (Button 2)
        0x4000, // X (Button 3)
        0x8000, // Y (Button 4)
        0x0010, // START (Button 5)
        0x0020, // BACK (Button 6)
        0x0040, // LSHOULDER (Button 7)
        0x0080, // RSHOULDER (Button 8)
        static_cast<uint16_t>(-1), // LTRIGGER
        static_cast<uint16_t>(-1), // RTRIGGER
        0x0100, // LTHUMB (Button 9)
        0x0200  // RTHUMB (Button 10)
    };
}