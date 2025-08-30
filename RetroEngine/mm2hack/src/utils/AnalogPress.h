//==============================================================================
// 
//  Project: mm2hack
//  ***.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <array>
#include <cmath>

namespace mm2hack::utils
{
    struct AxisPressParams
    {
        float deadzone = 0.20f;     // No input margin
        float pressThr = 0.60f;     // Press threshold
        float releaseThr = 0.45f;   // Release threshold
        float smoothA = 0.25f;      // Smoothing factor (0..1)
    };

    // 軸ごとに状態保持
    struct AxisPressState
    {
        float smoothed = 0.0f;
        bool  latchedPos = false;
        bool  latchedNeg = false;
    };

    inline float antiDeadzone(float v, float dz)
    {
        const float s = std::copysignf(1.0f, v);
        const float a = std::fabs(v);
        if (a <= dz) return 0.0f;
        return s * (a - dz) / (1.0f - dz);
    }
}