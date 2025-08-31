//==============================================================================
// 
//  Project: mm2hack
//  KeyToken.h
// 
//  Input device token definitions.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::input
{
    enum class Device : uint8_t { Keyboard = 0, XInput = 1, DirectInput = 2 };

    constexpr uint16_t kTokenUnbound = 0xFFFFui16;      // Unbound token value
    constexpr uint16_t kMaskType = 0xC000u;             // Mask to identify reserved tokens

    constexpr bool IsUnboundToken(uint16_t t) noexcept { return t == kTokenUnbound; }
    constexpr bool IsReservedToken(uint16_t t) noexcept { return (t & kMaskType) == kMaskType; }

    // Get a brief string representation of the token state
    inline const char* TokenBrief(uint16_t t) noexcept
    {
        return IsUnboundToken(t) ? "UNBOUND" : "BOUND";
    }
}