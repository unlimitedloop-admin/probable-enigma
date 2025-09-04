//==============================================================================
// 
//  Project: mm2hack
//  DirectInputToken.h
// 
//  DirectInput device token definitions.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "input/KeyToken.h"

namespace mm2hack::input::di
{
    // Upper 2 bits for type: 00=Button, 01=POV, 10=Axis
    constexpr uint16_t kTypeBtn  = 0x0000u;     // Buttons[code]
    constexpr uint16_t kTypeAxis = 0x8000u;     // X, Y, Z, Rx, Ry, Rz, Slider[0], Slider[1]
    constexpr uint16_t kTypePOV  = 0x4000u;     // code: 0=Up, 1=Right, 2=Down, 3=Left
    constexpr uint16_t kNegBit   = 0x1000u;     // Negative direction for Axis

    constexpr uint16_t MakeBtn(uint8_t idx) noexcept { return kTypeBtn | idx; }
    constexpr uint16_t MakeAxis(uint8_t axis/*0=X,1=Y,2=Z,3=Rx,4=Ry,5=Rz,6=S0,7=S1*/,
        bool negative, uint8_t thr)            noexcept
    {
        return kTypeAxis | (negative ? kNegBit : 0) | ((thr & 0x0F) << 8) | axis;
    }
    constexpr uint16_t MakePOV(uint8_t dir/*0=U,1=R,2=D,3=L*/) noexcept { return kTypePOV | dir; }

    constexpr bool     IsBtn(uint16_t t) noexcept { return (t & kMaskType) == kTypeBtn; }
    constexpr bool     IsAxis(uint16_t t) noexcept { return (t & kMaskType) == kTypeAxis; }
    constexpr bool     IsPOV(uint16_t t) noexcept { return (t & kMaskType) == kTypePOV; }
    constexpr uint8_t  Code(uint16_t t) noexcept { return static_cast<uint8_t>(t & 0xFFu); }
    constexpr uint8_t  ThrNib(uint16_t t) noexcept { return static_cast<uint8_t>((t >> 8) & 0x0Fu); }
    constexpr float    Thr01(uint16_t t) noexcept { return ThrNib(t) / 15.0f; }
    constexpr bool     IsNeg(uint16_t t) noexcept { return (t & kNegBit) != 0; }

    // Axis index
    constexpr uint8_t AX_X = 0, AX_Y = 1, AX_Z = 2, AX_Rx = 3, AX_Ry = 4, AX_Rz = 5, AX_S0 = 6, AX_S1 = 7;
    // POV direction (Up, Right, Down, Left)
    constexpr uint8_t POV_U = 0, POV_R = 1, POV_D = 2, POV_L = 3;
}