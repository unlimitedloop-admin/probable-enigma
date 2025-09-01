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
#include "Jpbtn.h"

namespace mm2hack::input
{
    // Definition of input device types
    enum class Device : uint8_t { Keyboard = 0, XInput = 1, DirectInput = 2 };

    // Definition of key/button tokens
    constexpr const wchar_t* kJpbtnKeys[JPBTN_COUNT] = {
        L"Up", L"Down", L"Left", L"Right",
        L"A", L"B", L"X", L"Y",
        L"Start", L"Back",
        L"LShoulder", L"RShoulder",
        L"LTrigger", L"RTrigger",
        L"LThumb", L"RThumb"
    };

    constexpr uint16_t kTokenUnbound = 0xFFFFui16;      // Unbound token value
    constexpr uint16_t kMaskType = 0xC000u;             // Mask to identify reserved tokens

    // Check if a token is unbound or reserved
    constexpr bool IsUnboundToken(uint16_t t) noexcept { return t == kTokenUnbound; }
    constexpr bool IsReservedToken(uint16_t t) noexcept { return (t & kMaskType) == kMaskType; }

    // Get a brief string representation of the token state
    inline const char* TokenBrief(uint16_t t) noexcept
    {
        return IsUnboundToken(t) ? "UNBOUND" : "BOUND";
    }
}