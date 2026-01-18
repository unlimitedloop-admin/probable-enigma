//==============================================================================
// 
//  Project: mm2hack
//  TileAttribute.h
// 
//  Tile attributes for the physics system.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <span>
#include <type_traits>

namespace mm2hack::apps::systems::physics
{
    // Tile attributes, bitfield
    enum class TileAttribute : std::uint16_t
    {
        None = 0,
        Solid             = 1 << 0,     // blockage
        Empty             = 1 << 1,     // space
        Ladder            = 1 << 2,     // ladder (space with event)
        InstantDeath      = 1 << 3,     // instant death (block with event)
        OneWayPlatform    = 1 << 4,     // one-way platform
        Water             = 1 << 5,     // water area (slow/float trigger)
        Damage            = 1 << 6,     // obstacle wall [extended blockage]
        ReflectProjectile = 1 << 7,     // reflect wall [extended blockage]
        NoCollision       = 1 << 8      // no collision (decoration)
    };

    // Setting of tile attribute over a range of tiles
    struct TileAttributeRange
    {
        int firstTile;                  // Beginning tile index (inclusive)
        int lastTile;                   // Bounding tile index (inclusive)
        TileAttribute attr;             // Attribute to apply
    };

    // Applies the specified tile attributes to all tiles within the given ranges using the provided TileManager
    template <class TileManager>
    void ApplyTileAttributeRanges(TileManager& manager, std::span<const TileAttributeRange> ranges)
    {
        for (const auto& r : ranges)
        {
            for (int t = r.firstTile; t <= r.lastTile; ++t)
            {
                manager.SetTileAttribute(t, r.attr);
            }
        }
    }

    // Checks if the specified tile attribute flag is set in the given tile attribute value
    [[nodiscard]] constexpr bool Has(const TileAttribute v, const TileAttribute f)
    {
        using U = std::underlying_type_t<TileAttribute>;
        return (static_cast<U>(v) & static_cast<U>(f)) != 0;
    }

    // Bitwise OR operator overload for TileAttribute enum class
    [[nodiscard]] constexpr TileAttribute operator|(TileAttribute a, TileAttribute b)
    {
        using U = std::underlying_type_t<TileAttribute>;
        return static_cast<TileAttribute>(static_cast<U>(a) | static_cast<U>(b));
    }
}