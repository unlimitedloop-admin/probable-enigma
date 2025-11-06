#include "pch.h"

#include "MapPageCache.h"

#include "AddressScraper.h"
#include "cstring"

namespace
{
    using conf = mm2hack::config::SystemConfig;
    constexpr std::size_t kPageSize = conf::kMapBinaryUnitPageSize;
    constexpr std::size_t kPayloadOff = conf::kMapBinaryHeaderSize;
}


namespace mm2hack::apps::resources::bg
{
    static_assert(PageTiles::kSize == 240, "it must be 240(16x15=240)");

    void MapPageCache::BuildAround(const std::size_t currentPageIndex)
    {
        // The ensure function loads the page if the index is valid.
        auto ensure = [&](std::optional<std::size_t> idxOpt)
            {
                if (!idxOpt) return;
                const auto idx = *idxOpt;
                _cache[idx] = readTiles_(idx);
            };

        _cache[currentPageIndex] = readTiles_(currentPageIndex);

        // Neighbors: horizontal and vertical
        const auto r = Right(currentPageIndex);
        const auto l = Left(currentPageIndex);
        const auto u = Up(currentPageIndex);
        const auto d = Down(currentPageIndex);

        ensure(r); ensure(l); ensure(u); ensure(d);

        // Diagonal: horizontal -> vertical (only if both exist)
        if (r && d) ensure(RightDown(currentPageIndex));
        if (l && d) ensure(LeftDown(currentPageIndex));
        if (r && u) ensure(RightUp(currentPageIndex));
        if (l && u) ensure(LeftUp(currentPageIndex));
    }

    std::uint8_t MapPageCache::Tile(const std::size_t pageIndex, const int tx, const int ty) const
    {
        if (tx < 0 || ty < 0 || tx >= PageTiles::kW || ty >= PageTiles::kH) return 0;

        auto it = _cache.find(pageIndex);
        if (it == _cache.end())
        {
            // (Lazy loading) Safe to reference even if BuildAround was forgotten
            auto inserted = _cache.emplace(pageIndex, readTiles_(pageIndex));
            it = inserted.first;
        }

        const auto& cells = it->second.cells;
        return cells[static_cast<std::size_t>(ty) * PageTiles::kW + static_cast<std::size_t>(tx)];
    }

    // Neighbor resolution: AddressScraper is assumed to return roomNo for pageIndex.
    // From there, pageIndex is re-resolved.
    std::optional<std::size_t> MapPageCache::Right(const std::size_t page) const
    {
        const int16_t roomNo = _s.getRightRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex_(idx);
    }

    std::optional<std::size_t> MapPageCache::Left(const std::size_t page) const
    {
        const int16_t roomNo = _s.getLeftRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex_(idx);
    }

    std::optional<std::size_t> MapPageCache::Up(const std::size_t page) const
    {
        const int16_t roomNo = _s.getOverRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex_(idx);
    }

    std::optional<std::size_t> MapPageCache::Down(const std::size_t page) const
    {
        const int16_t roomNo = _s.getUnderRoom(static_cast<int>(page));
        const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
        return toOptIndex_(idx);
    }

    std::optional<std::size_t> MapPageCache::RightDown(const std::size_t page) const
    {
        if (auto r = Right(page))
        {
            const int16_t roomNo = _s.getUnderRoom(static_cast<int>(*r));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex_(idx);
        }
        return std::nullopt;
    }

    std::optional<std::size_t> MapPageCache::LeftDown(const std::size_t page) const
    {
        if (auto l = Left(page))
        {
            const int16_t roomNo = _s.getUnderRoom(static_cast<int>(*l));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex_(idx);
        }
        return std::nullopt;
    }

    std::optional<std::size_t> MapPageCache::RightUp(const std::size_t page) const
    {
        if (auto r = Right(page))
        {
            const int16_t roomNo = _s.getOverRoom(static_cast<int>(*r));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex_(idx);
        }
        return std::nullopt;
    }

    std::optional<std::size_t> MapPageCache::LeftUp(const std::size_t page) const
    {
        if (auto l = Left(page))
        {
            const int16_t roomNo = _s.getOverRoom(static_cast<int>(*l));
            const int16_t idx = _s.getPageIndex(static_cast<std::size_t>(roomNo));
            return toOptIndex_(idx);
        }
        return std::nullopt;
    }

    PageTiles MapPageCache::readTiles_(const std::size_t pageIndex) const
    {
        const auto& bin = _s.GetBin();

        const std::size_t off = pageIndex * kPageSize + kPayloadOff;
        const std::size_t need = PageTiles::kSize; // 240
        PageTiles tiles{};

        if (off + need <= bin.size())
        {
            // Copy tile data to PageTiles (left -> right, top -> bottom)
            std::memcpy(tiles.cells.data(), bin.data() + off, need);
        }
        else
        {
            // Insufficient size: return as-is for safety
        }
        return tiles;
    }
}