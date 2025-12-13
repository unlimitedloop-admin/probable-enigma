//==============================================================================
// 
//  Project: mm2hack
//  Probes.h
// 
//  It has points for detecting contact with objects.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::systems::physics
{
    struct BehindGroundProbe
    {
        foundation::math::Vec2 topPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint{ 0.0, 0.0 };
    };

    struct FrontLineProbe
    {
        foundation::math::Vec2 topPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint{ 0.0, 0.0 };
    };

    struct RearLineProbe
    {
        foundation::math::Vec2 topPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint{ 0.0, 0.0 };
    };

    struct TopLineProbe
    {
        foundation::math::Vec2 frontPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 behindPoint{ 0.0, 0.0 };
    };

    struct BottomLineProbe
    {
        foundation::math::Vec2 frontPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 behindPoint{ 0.0, 0.0 };
    };

    // The probes container for character physics
    struct Probes
    {
        BehindGroundProbe behindGround{};
        FrontLineProbe    frontLine{};
        RearLineProbe     rearLine{};
        TopLineProbe      topLine{};
        BottomLineProbe   bottomLine{};

        // Reset all probes to zero
        void Reset() noexcept
        {
            behindGround = {};
            frontLine   = {};
            rearLine    = {};
            topLine     = {};
            bottomLine  = {};
        }
    };
}