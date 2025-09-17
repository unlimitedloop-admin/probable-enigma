//==============================================================================
// 
//  Project: mm2hack
//  Jpbtn.h
// 
//  Joystick Button Enumeration.
// 
//==============================================================================
#pragma once

namespace mm2hack
{
    // Enum representing the buttons on a Joystick or gamepad
    enum class JPBTN
    {
        UP = 0,
        DOWN,
        LEFT,
        RIGHT,
        A,
        B,
        X,
        Y,
        START,
        BACK,
        LSHOULDER,
        RSHOULDER,
        LTRIGGER,
        RTRIGGER,
        LTHUMB,
        RTHUMB,

        JPBTN_COUNT // Total number of buttons
    };
}

constexpr size_t JPBTN_COUNT = static_cast<size_t>(mm2hack::JPBTN::JPBTN_COUNT);
