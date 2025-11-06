//==============================================================================
// 
//  Project: mm2hack
//  WorldTileQuery.h
// 
//  Helper in querying world tile attributes based on world pixel coordinates.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <string>

namespace mm2hack::apps::resources::bg
{
    class MapPageCache;
}

namespace mm2hack::apps::systems::scrolling::atomic
{
    struct Camera;
}

namespace mm2hack::apps::systems::scrolling::atomic
{
    // World tile query helper
    class WorldTileQuery
    {
        using MapPageCache = apps::resources::bg::MapPageCache;

    public:
        WorldTileQuery(MapPageCache& cache, size_t currentPage, int tilePx)
            : _cache(cache), _curr(currentPage), _ts(tilePx)
        {
        }

        // Set current page index
        void SetCurrentPage(size_t p) { _curr = p; }
        // Get tile attribute at the given world pixel position
        uint8_t TileAtPx(double worldPxX, double worldPxY, const Camera& cam) const;

    private:
        const std::wstring kClassName = L"WorldTileQuery";

        MapPageCache& _cache;
        size_t _curr;
        int _ts;
    };
}