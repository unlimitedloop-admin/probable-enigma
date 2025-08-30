//==============================================================================
// 
//  Project: mm2hack
//  ***.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::input::xi
{

    // Upper 2 bits for type: 00=Button, 01=Trigger, 10=Axis
    constexpr uint16_t kMaskType = 0xC000u;
    constexpr uint16_t kTypeBtn  = 0x0000u;
    constexpr uint16_t kTypeTrg  = 0x4000u;
    constexpr uint16_t kTypeAxis = 0x8000u;

    // For Axis: sign bit (negative direction), Common: threshold nibble (0..15)
    constexpr uint16_t kNegBit   = 0x1000u;

    constexpr uint16_t MakeBtn(uint8_t btnIdx) noexcept { return kTypeBtn | btnIdx; }
    constexpr uint16_t MakeTrig(uint8_t which/*0=LT,1=RT*/, uint8_t thr) noexcept { return kTypeTrg | ((thr & 0x0F) << 8) | which; }
    constexpr uint16_t MakeAxis(uint8_t axis/*0=LX,1=LY,2=RX,3=RY*/, bool negative, uint8_t thr) noexcept
    {
        return kTypeAxis | (negative ? kNegBit : 0) | ((thr & 0x0F) << 8) | axis;
    }

    constexpr bool     IsBtn(uint16_t t) noexcept { return (t & kMaskType) == kTypeBtn; }
    constexpr bool     IsTrig(uint16_t t) noexcept { return (t & kMaskType) == kTypeTrg; }
    constexpr bool     IsAxis(uint16_t t) noexcept { return (t & kMaskType) == kTypeAxis; }
    constexpr uint8_t  Code(uint16_t t) noexcept { return static_cast<uint8_t>(t & 0xFFu); }
    constexpr uint8_t  ThrNib(uint16_t t) noexcept { return static_cast<uint8_t>((t >> 8) & 0x0Fu); }
    constexpr float    Thr01(uint16_t t) noexcept { return ThrNib(t) / 15.0f; }
    constexpr bool     IsNeg(uint16_t t) noexcept { return (t & kNegBit) != 0; }

    // Constants
    constexpr uint8_t LT = 0;
    constexpr uint8_t RT = 1;
    constexpr uint8_t LX = 0, LY = 1, RX = 2, RY = 3;

}