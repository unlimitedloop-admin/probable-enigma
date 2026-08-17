//==============================================================================
//
//  Project: mm2hack
//  StageTileAnimations.h
//
//  Configuration of stage BG tile animations.
//
//==============================================================================
#pragma once

#include <array>
#include <span>

#include "apps/rendering/bg/BGTileAnimator.h"

namespace mm2hack::apps::resources::stages
{
    // Water surface left tile:
    // 130 -> 144 -> 146 -> ...
    inline constexpr std::array<rendering::bg::BGTileAnimationFrame, 3>
        STAGE2_WATER_SURFACE_A
    {
        rendering::bg::BGTileAnimationFrame{ 130, 12 },
        rendering::bg::BGTileAnimationFrame{ 144, 12 },
        rendering::bg::BGTileAnimationFrame{ 146, 12 }
    };

    // Water surface right tile:
    // 131 -> 145 -> 147 -> ...
    inline constexpr std::array<rendering::bg::BGTileAnimationFrame, 3>
        STAGE2_WATER_SURFACE_B
    {
        rendering::bg::BGTileAnimationFrame{ 131, 12 },
        rendering::bg::BGTileAnimationFrame{ 145, 12 },
        rendering::bg::BGTileAnimationFrame{ 147, 12 }
    };

    // Stage 2 BG tile animations
    inline constexpr std::array<rendering::bg::BGTileAnimation, 2> STAGE2_TILEANIMATIONS
    {
        rendering::bg::BGTileAnimation{
            130,
            std::span<const rendering::bg::BGTileAnimationFrame>{
                STAGE2_WATER_SURFACE_A }
        },
        rendering::bg::BGTileAnimation{
            131,
            std::span<const rendering::bg::BGTileAnimationFrame>{
                STAGE2_WATER_SURFACE_B }
        }
    };
}