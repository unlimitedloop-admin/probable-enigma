//==============================================================================
// 
//  Project: mm2hack
//  MapPageCache.h
// 
//  The `MapPageCache` class provides a lightweight, lazy-loading cache of map page tile data for the background system.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include "config/SystemConfig.h"

namespace mm2hack::apps::resources::bg
{
    class AddressScraper;
}

namespace mm2hack::apps::resources::bg
{
    // One page's tile data (16x15=240 tiles)
    struct PageTiles
    {
        static constexpr int kW = config::SystemConfig::kTileCountX;
        static constexpr int kH = config::SystemConfig::kTileCountY;
        static constexpr int kSize = kW * kH;       // 240
        std::array<std::uint8_t, kSize> cells{};
    };

    // Map page cache with lazy loading
    class MapPageCache
    {
    public:
        explicit MapPageCache(AddressScraper& s) : _s(s) {}

        // (Lazy loading) Load the specified page and its surrounding 8 pages into the cache
        void BuildAround(std::size_t currentPageIndex);

        // (Lazy loading) Return the tile ID at (tx, ty) of the specified page. Out of range returns 0
        std::uint8_t Tile(std::size_t pageIndex, int tx, int ty) const;

        // Neighbor resolution (returns nullopt if not found)
        std::optional<std::size_t> Right(std::size_t page) const;
        std::optional<std::size_t> Left(std::size_t page) const;
        std::optional<std::size_t> Up(std::size_t page) const;
        std::optional<std::size_t> Down(std::size_t page) const;

        std::optional<std::size_t> RightDown(std::size_t page) const;
        std::optional<std::size_t> LeftDown(std::size_t page) const;
        std::optional<std::size_t> RightUp(std::size_t page) const;
        std::optional<std::size_t> LeftUp(std::size_t page) const;

    private:
        PageTiles readTiles_(std::size_t pageIndex) const;      // Read tile data from AddressScraper

        static std::optional<std::size_t> toOptIndex_(int16_t idx)
        {
            return (idx >= 0) ? std::optional<std::size_t>(static_cast<std::size_t>(idx)) : std::nullopt;
        }   // Convert -1 to nullopt

    private:
        const std::wstring kClassName = L"MapPageCache";

        AddressScraper& _s;
        mutable std::unordered_map<std::size_t, PageTiles> _cache;  // pageIndex -> PageTiles
    };
}