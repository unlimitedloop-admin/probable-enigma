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
        KEY_INPUT_W,                    // UP
        KEY_INPUT_S,                    // DOWN
        KEY_INPUT_A,                    // LEFT
        KEY_INPUT_D,                    // RIGHT
        KEY_INPUT_M,                    // A
        KEY_INPUT_N,                    // B
        KEY_INPUT_L,                    // X
        KEY_INPUT_K,                    // Y
        KEY_INPUT_RETURN,               // START
        KEY_INPUT_SPACE,                // BACK
        KEY_INPUT_1,                    // LSHOULDER
        KEY_INPUT_0,                    // RSHOULDER
        KEY_INPUT_5,                    // LTRIGGER
        KEY_INPUT_7,                    // RTRIGGER
        KEY_INPUT_F,                    // LTHUMB
        KEY_INPUT_H                     // RTHUMB
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
        static_cast<uint16_t>(-1),      // UP: for POV
        static_cast<uint16_t>(-1),      // DOWN
        static_cast<uint16_t>(-1),      // LEFT
        static_cast<uint16_t>(-1),      // RIGHT
        0,                              // A
        1,                              // B
        3,                              // X
        4,                              // Y
        11,                             // START
        10,                             // BACK
        6,                              // LB
        7,                              // RB
        8,                              // LT (May not be present on some devices)
        9,                              // RT (ditto)
        13,                             // LTHUMB
        14                              // RTHUMB
    };
    // NOTE: Buttons are 0-based indices. Axes and POVs are handled separately.
}