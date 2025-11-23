//==============================================================================
// 
//  Project: mm2hack
//  Probes.h
// 
//  It has points for detecting contact with objects.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::systems::physics
{
    struct BehindGroundProbe
    {
        std::uint8_t topPoint{ 0 };
        std::uint8_t middlePoint{ 0 };
        std::uint8_t bottomPoint{ 0 };
    };

    struct FrontLineProbe
    {
        std::uint8_t topPoint{ 0 };
        std::uint8_t middlePoint{ 0 };
        std::uint8_t bottomPoint{ 0 };
    };

    struct RearLineProbe
    {
        std::uint8_t topPoint{ 0 };
        std::uint8_t middlePoint{ 0 };
        std::uint8_t bottomPoint{ 0 };
    };

    struct TopLineProbe
    {
        std::uint8_t leftPoint{ 0 };
        std::uint8_t middlePoint{ 0 };
        std::uint8_t rightPoint{ 0 };
    };

    struct BottomLineProbe
    {
        std::uint8_t leftPoint{ 0 };
        std::uint8_t middlePoint{ 0 };
        std::uint8_t rightPoint{ 0 };
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