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

namespace mm2hack::input
{
    // 16bit Token Layout:
    // [15..14] Device: 00=Keyboard, 01=XInput, 10=DirectInput, 11=Reserved
    // [13]     Analog: 0=Button/Key, 1=Axis/Trigger
    // [12]     Polarity: 0=Positive/Right/Down, 1=Negative/Left/Up  (Triggersは常に0)
    // [11..8]  Threshold nibble (0..15) 例: 8=約50%
    // [7..0]   Code (ボタン/キー/軸のインデックス)
    enum class Device : uint8_t { Keyboard = 0, XInput = 1, DirectInput = 2 };

    constexpr uint16_t MakeToken(Device dev, bool analog, uint8_t code,
        bool negative = false, uint8_t thrNibble = 8) noexcept
    {
        return (static_cast<uint16_t>(dev) << 14) |
            (static_cast<uint16_t>(analog) << 13) |
            (static_cast<uint16_t>(negative) << 12) |
            ((static_cast<uint16_t>(thrNibble) & 0x0F) << 8) |
            (static_cast<uint16_t>(code));
    }

    constexpr Device GetDevice(uint16_t t)     noexcept { return static_cast<Device>((t >> 14) & 0x3); }
    constexpr bool   IsAnalog(uint16_t t)      noexcept { return ((t >> 13) & 0x1) != 0; }
    constexpr bool   IsNegative(uint16_t t)    noexcept { return ((t >> 12) & 0x1) != 0; }
    constexpr uint8_t ThrNibble(uint16_t t)    noexcept { return static_cast<uint8_t>((t >> 8) & 0x0F); }
    constexpr uint8_t Code(uint16_t t)         noexcept { return static_cast<uint8_t>(t & 0xFF); }

    // しきい値を正規化（0..1）
    constexpr float Thr01(uint16_t t) noexcept { return static_cast<float>(ThrNibble(t)) / 15.0f; }

    // XInput コード規約（簡潔に）：Buttons[]のインデックスを 0..15、
    // Triggers: 0=LT, 1=RT、Axes: 0=LX,1=LY,2=RX,3=RY（極性は Polarity で指定）
    enum : uint8_t
    {
        XI_LT = 0, XI_RT = 1,  // Triggers when Analog==true
        XI_LX = 0, XI_LY = 1, XI_RX = 2, XI_RY = 3 // Axes when Analog==true
    };
}