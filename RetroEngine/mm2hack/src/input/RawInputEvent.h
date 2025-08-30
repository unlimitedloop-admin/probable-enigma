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
    enum class RawDevice : uint8_t { Keyboard, XInput, DirectInput };
    enum class RawKind : uint8_t { Button, Trigger, Axis, Key };

    struct RawInputEvent
    {
        RawDevice device;
        RawKind   kind;
        uint8_t   code;     // Button idx / Axis idx / Trigger idx / VK
        bool      negative; // Axis のみ
        float     value;    // 正規化 0..1（Axisは絶対値、Triggerは0..1）
    };
}
