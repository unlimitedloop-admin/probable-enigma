//==============================================================================
// 
//  Project: mm2hack
//  BgTileQueryHelper.h
// 
//  Helper in querying world tile attributes based on world pixel coordinates.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::resources::bg
{
    class MapPageCache;
}

namespace mm2hack::apps::systems::physics
{
    // Background tile query helper
    class BgTileQueryHelper
    {
        using MapPageCache = resources::bg::MapPageCache;
        using Camera       = scrolling::atomic::Camera;

    public:
        BgTileQueryHelper(MapPageCache& cache, size_t currentPage, int tilePx)
            : _cache(cache), _curr(currentPage), _ts(tilePx)
        {
        }

        // Set current page index
        void SetCurrentPage(size_t p) { _curr = p; }
        // Get tile attribute at the given world pixel position
        uint8_t TileAtPx(double worldPxX, double worldPxY, const Camera& cam) const;

    private:
        const std::wstring kClassName{L"BgTileQueryHelper"};

        const int tSizeW = config::SystemConfig::kTileCountX;
        const int tSizeH = config::SystemConfig::kTileCountY;

        MapPageCache& _cache;
        size_t _curr;
        int _ts;
    };
}