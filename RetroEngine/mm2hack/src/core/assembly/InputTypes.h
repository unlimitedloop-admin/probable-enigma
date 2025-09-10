//==============================================================================
// 
//  Project: mm2hack
//  InputTypes.h
// 
//  Input types for the input system.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include "input/Jpbtn.h"

namespace mm2hack::core::assembly
{
    using Key16 = JPBTN;    // Logical key ID (JPBTN alias)

    // State of a single key for the current frame
    struct KeyFrameState
    {
        bool pressed{};         // Is pressed this tick
        std::int32_t frames{};  // Continuous frame count: +N while pressed / -N while not pressed
        bool changed{};         // Did an edge occur this tick (JustPressed/Released)
    };

    // Snapshot of the entire input state at a specific tick
    struct InputSnapshot
    {
        std::array<KeyFrameState, static_cast<size_t>(Key16::JPBTN_COUNT)> keys{};
        std::uint64_t tick{};   // Simulation tick number (can remain 0 if not needed)
    };
}