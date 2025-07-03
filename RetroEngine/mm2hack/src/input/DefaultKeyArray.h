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