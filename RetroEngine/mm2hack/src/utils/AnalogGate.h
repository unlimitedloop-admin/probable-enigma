//==============================================================================
// 
//  Project: mm2hack
//  ***.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <chrono>

namespace mm2hack::utils
{
    struct AxisCaptureParams
    {
        float deadzone = 0.25f;     // Just a noise margin
        float pressThr = 0.60f;     // Press threshold
        int   holdMs = 120;         // Press hold time
        int   settleMs = 120;       // Settle time after release
    };

    struct AxisSnapshot
    {
        float cx = 0.0f, cy = 0.0f; // Offset corrected current value
        std::chrono::steady_clock::time_point t0{};
    };
}