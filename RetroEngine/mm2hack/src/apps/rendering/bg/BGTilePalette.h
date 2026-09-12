//==============================================================================
//
//  Project: mm2hack
//  BGTilePalette.h
//
//  Definitions for per-tile BG palette variants.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <span>

namespace mm2hack::apps::rendering::bg
{
    // Maps one PNG palette index to one NES palette color.
    struct BGPaletteColorMapping
    {
        std::uint8_t pngPaletteIndex{ 0 };
        std::uint8_t nesPaletteIndex{ 0 };
    };

    // One palette variant for a specific BG tile.
    struct BGTilePaletteVariant
    {
        std::span<const BGPaletteColorMapping> mappings{};
    };
}