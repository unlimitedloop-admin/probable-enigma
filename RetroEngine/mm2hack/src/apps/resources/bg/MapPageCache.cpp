#include "pch.h"

#include "MapPageCache.h"

#include "AddressScraper.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"

namespace mm2hack::apps::resources::bg
{
    using systems::scrolling::atomic::ScrollKind;

    MapPageCache::MapPageCache(std::shared_ptr<AddressScraper> scraper)
        : _scraper(std::move(scraper))
    {
    }

    void MapPageCache::BuildAround(std::size_t currentPageIndex)
    {
        // Simple eager load of current and 8-neighbors if present
        const std::array<int, 9> offs = { 0, -1, 1, -16, 16, -17, -15, 15, 17 }; // heuristic; real mapping depends on layout
        for (auto o : offs)
        {
            std::size_t idx = currentPageIndex;
            if (o < 0)
            {
                if (static_cast<std::size_t>(-o) > currentPageIndex) continue;
                idx = currentPageIndex - static_cast<std::size_t>(-o);
            }
            else
            {
                idx = currentPageIndex + static_cast<std::size_t>(o);
            }

            if (_cache.find(idx) == _cache.end())
            {
                _cache.emplace(idx, readTiles_(idx));
            }
        }
    }

    PageTiles MapPageCache::readTiles_(std::size_t pageIndex) const
    {
        PageTiles out{};
        if (!_scraper) return out;
        if (pageIndex >= _scraper->pageCount()) return out;

        const std::uint8_t* p = _scraper->payloadPtr(pageIndex);
        if (!p) return out;

        for (int i = 0; i < PageTiles::kSize; ++i)
        {
            out.cells[i] = p[i];
        }
        return out;
    }

    std::uint8_t MapPageCache::GetTile(std::size_t pageIndex, int tx, int ty) const
    {
        if (tx < 0 || ty < 0 || tx >= PageTiles::kW || ty >= PageTiles::kH) return 0;
        auto it = _cache.find(pageIndex);
        if (it == _cache.end())
        {
            const auto tiles = readTiles_(pageIndex);
            it = _cache.emplace(pageIndex, tiles).first;
        }
        const auto& cells = it->second.cells;
        const int idx = ty * PageTiles::kW + tx;
        return cells[idx];
    }

    std::optional<ScrollKind> MapPageCache::ScrollTypeRight(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        const int16_t v = _scraper->getRightScrollType(pageIndex);
        return static_cast<ScrollKind>(v);
    }
    std::optional<ScrollKind> MapPageCache::ScrollTypeLeft(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        const int16_t v = _scraper->getLeftScrollType(pageIndex);
        return static_cast<ScrollKind>(v);
    }
    std::optional<ScrollKind> MapPageCache::ScrollTypeUp(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        const int16_t v = _scraper->getOverScrollType(pageIndex);
        return static_cast<ScrollKind>(v);
    }
    std::optional<ScrollKind> MapPageCache::ScrollTypeDown(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        const int16_t v = _scraper->getUnderScrollType(pageIndex);
        return static_cast<ScrollKind>(v);
    }

    std::optional<std::size_t> MapPageCache::NeighborRight(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        return toOptIndex_(_scraper->getRightRoom(pageIndex));
    }
    std::optional<std::size_t> MapPageCache::NeighborLeft(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        return toOptIndex_(_scraper->getLeftRoom(pageIndex));
    }
    std::optional<std::size_t> MapPageCache::NeighborUp(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        return toOptIndex_(_scraper->getOverRoom(pageIndex));
    }
    std::optional<std::size_t> MapPageCache::NeighborDown(std::size_t pageIndex) const
    {
        if (!_scraper) return std::nullopt;
        return toOptIndex_(_scraper->getUnderRoom(pageIndex));
    }

    std::optional<std::size_t> MapPageCache::RoomToPageIndex(uint8_t room) const
    {
        if (!_scraper) return std::nullopt;
        const int16_t idx = _scraper->getPageIndex(static_cast<std::size_t>(room));
        return toOptIndex_(idx);
    }

    int MapPageCache::TileSize() const
    {
        return PageTiles::kW; // tile size per tile (in px?) — keep simple, callers expect tile px maybe use config
    }

    int MapPageCache::MapWidth() const
    {
        // Best-effort: return page count in X * page tile width. If AddressScraper can't provide, return single page width.
        if (!_scraper) return PageTiles::kW;
        const std::size_t pageCount = _scraper->pageCount();
        // Assume a single-row layout if unknown
        return static_cast<int>(PageTiles::kW * std::max<std::size_t>(1, pageCount));
    }

    int MapPageCache::MapHeight() const
    {
        if (!_scraper) return PageTiles::kH;
        // naive
        return PageTiles::kH;
    }
}