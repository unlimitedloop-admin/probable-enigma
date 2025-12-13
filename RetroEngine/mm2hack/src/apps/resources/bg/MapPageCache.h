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
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "config/SystemConfig.h"
#include "IMapPageSource.h"

namespace mm2hack::apps::resources::bg
{
    class AddressScraper;

    // One page's tile data (16x15=240 tiles)
    struct PageTiles
    {
        static constexpr int kW = config::SystemConfig::kTileCountX;
        static constexpr int kH = config::SystemConfig::kTileCountY;
        static constexpr int kSize = kW * kH;       // 240
        std::array<std::uint8_t, kSize> cells{};
    };

    // MapPageCache: Cache of map page tile data with lazy loading
    class MapPageCache final : public IMapPageSource
    {
        using ScrollKind = systems::scrolling::atomic::ScrollKind;

    public:
        explicit MapPageCache(std::shared_ptr<AddressScraper> scraper);

        // Lazy-loading helper
        void BuildAround(std::size_t currentPageIndex);

        // IMapPageSource implementation
        std::uint8_t GetTile(std::size_t pageIndex, int tx, int ty) const override;

        // Get scroll type for each direction (returns std::nullopt if not scrollable)
        [[nodiscard]] std::optional<systems::scrolling::atomic::ScrollKind> ScrollTypeRight(std::size_t pageIndex) const override;
        [[nodiscard]] std::optional<systems::scrolling::atomic::ScrollKind> ScrollTypeLeft(std::size_t pageIndex) const override;
        [[nodiscard]] std::optional<systems::scrolling::atomic::ScrollKind> ScrollTypeUp(std::size_t pageIndex) const override;
        [[nodiscard]] std::optional<systems::scrolling::atomic::ScrollKind> ScrollTypeDown(std::size_t pageIndex) const override;

        // Get neighboring page index for each direction (returns std::nullopt if no neighbor)
        [[nodiscard]] std::optional<std::size_t> NeighborRight(std::size_t pageIndex) const override;
        [[nodiscard]] std::optional<std::size_t> NeighborLeft(std::size_t pageIndex) const override;
        [[nodiscard]] std::optional<std::size_t> NeighborUp(std::size_t pageIndex) const override;
        [[nodiscard]] std::optional<std::size_t> NeighborDown(std::size_t pageIndex) const override;

        // map room id -> page index
        [[nodiscard]] std::optional<std::size_t> RoomToPageIndex(uint8_t room) const override;

        // Get the overall pixel size, stage map size
        [[nodiscard]] int TileSize() const override;
        [[nodiscard]] int MapWidth() const override;
        [[nodiscard]] int MapHeight() const override;

    private:
        PageTiles readTiles_(std::size_t pageIndex) const;      // Read tile data from AddressScraper

        static std::optional<std::size_t> toOptIndex_(int16_t idx)
        {
            return (idx >= 0) ? std::optional<std::size_t>(static_cast<std::size_t>(idx)) : std::nullopt;
        }

    private:
        const std::wstring kClassName{ L"MapPageCache" };

        std::shared_ptr<AddressScraper> _scraper;   // OWN HOLDER! (Shared to ScraperScrollRuleProvider etc.)
        mutable std::unordered_map<std::size_t, PageTiles> _cache;  // pageIndex -> PageTiles
    };
}