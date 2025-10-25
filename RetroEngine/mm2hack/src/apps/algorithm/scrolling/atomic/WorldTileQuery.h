//==============================================================================
// 
//  Project: mm2hack
//  WorldTileQuery.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include "apps/algorithm/scrolling/atomic/Scroll.h"
#include "apps/graphics/bg/MapPageCache.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    class WorldTileQuery
    {
        using MapPageCache = graphics::bg::MapPageCache;

    public:
        WorldTileQuery(MapPageCache& cache, size_t currentPage, int tilePx)
            : _cache(cache), _curr(currentPage), _ts(tilePx)
        {
        }

        void SetCurrentPage(size_t p) { _curr = p; }

        // 画面上のワールドpx（= 物体の世界座標）とカメラを受け取り、正しいページから tile を得る
        uint8_t TileAtPx(double worldPxX, double worldPxY, const Camera& cam) const
        {
            const int pageW = 16 * _ts;
            const int pageH = 15 * _ts;

            // 「現在ページの左上」を世界座標系で 0 とみなす
            const double localX = worldPxX + cam.x;
            const double localY = worldPxY + cam.y;

            // local が [0,pageW] に収まっていなければ、どの隣接ページかを選ぶ
            size_t page = _curr;
            int     txOffset = 0, tyOffset = 0;

            double x = localX, y = localY;
            if (x < 0) { if (auto p = _cache.Left(_curr))  page = *p; x += pageW; }
            else if (x >= pageW) { if (auto p = _cache.Right(_curr)) page = *p; x -= pageW; }

            if (y < 0) { if (auto p = _cache.Up(_curr))    page = *p; y += pageH; }
            else if (y >= pageH) { if (auto p = _cache.Down(_curr))  page = *p; y -= pageH; }

            const int tx = std::clamp(static_cast<int>(std::floor(x / _ts)), 0, 15);
            const int ty = std::clamp(static_cast<int>(std::floor(y / _ts)), 0, 14);
            return _cache.Tile(page, tx, ty);
        }

    private:
        MapPageCache& _cache;
        size_t _curr;
        int _ts;
    };
}