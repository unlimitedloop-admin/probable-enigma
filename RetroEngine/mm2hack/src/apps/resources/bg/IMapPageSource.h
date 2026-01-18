//==============================================================================
//
//  Project: mm2hack
//  IMapPageSource.h
// 
//  Represents tile and adjacency for map pages in the BG system.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include "apps/systems/scrolling/atomic/ScrollTypes.h"

namespace mm2hack::apps::resources::bg
{
    // Abstract: Tile acquisition and adjacency information on a per-page basis
    class IMapPageSource
    {
        using ScrollKind = systems::scrolling::atomic::ScrollKind;

    public:
        virtual ~IMapPageSource() = default;

        // Get tile within the page (policy such as returning 0 if tx,ty are out of range)
        virtual std::uint8_t GetTile(std::size_t pageIndex, int tx, int ty) const = 0;

        // Get scroll type for each direction (returns std::nullopt if not scrollable)
        [[nodiscard]] virtual std::optional<ScrollKind> ScrollTypeRight(std::size_t pageIndex) const = 0;
        // Get scroll type for each direction (returns std::nullopt if not scrollable)
        [[nodiscard]] virtual std::optional<ScrollKind> ScrollTypeLeft(std::size_t pageIndex) const = 0;
        // Get scroll type for each direction (returns std::nullopt if not scrollable)
        [[nodiscard]] virtual std::optional<ScrollKind> ScrollTypeUp(std::size_t pageIndex) const = 0;
        // Get scroll type for each direction (returns std::nullopt if not scrollable)
        [[nodiscard]] virtual std::optional<ScrollKind> ScrollTypeDown(std::size_t pageIndex) const = 0;

        // Get neighboring page index for each direction (returns std::nullopt if no neighbor)
        [[nodiscard]] virtual std::optional<std::size_t> NeighborRight(std::size_t pageIndex) const = 0;
        // Get neighboring page index for each direction (returns std::nullopt if no neighbor)
        [[nodiscard]] virtual std::optional<std::size_t> NeighborLeft(std::size_t pageIndex) const = 0;
        // Get neighboring page index for each direction (returns std::nullopt if no neighbor)
        [[nodiscard]] virtual std::optional<std::size_t> NeighborUp(std::size_t pageIndex) const = 0;
        // Get neighboring page index for each direction (returns std::nullopt if no neighbor)
        [[nodiscard]] virtual std::optional<std::size_t> NeighborDown(std::size_t pageIndex) const = 0;

        // Map room id -> page index (returns nullopt if not found)
        [[nodiscard]] virtual std::optional<std::size_t> RoomToPageIndex(uint8_t room) const = 0;

        // Get the overall pixel size of the map (if needed)
        virtual int TileSize() const = 0;
        // Get the overall map width in tiles
        virtual int MapWidth() const = 0;
        // Get the overall map height in tiles
        virtual int MapHeight() const = 0;
    };
}