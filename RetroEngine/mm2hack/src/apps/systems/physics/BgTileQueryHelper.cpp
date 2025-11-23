#include "pch.h"

#include "BgTileQueryHelper.h"

#include <cmath>
#include "apps/resources/bg/MapPageCache.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"

namespace mm2hack::apps::systems::physics
{
    using scrolling::atomic::Camera;

    uint8_t BgTileQueryHelper::TileAtPx(double worldPxX, double worldPxY, const Camera& cam) const
    {
        const int pageW = tSizeW * _ts;
        const int pageH = tSizeH * _ts;

        // Coordinates base point is camera top-left.
        const double localX = worldPxX + cam.x;
        const double localY = worldPxY + cam.y;

        // If local is not in [0,pageW], choose adjacent page.
        size_t page = _curr;
        int txOffset = 0, tyOffset = 0;

        double x = localX, y = localY;
        if (x < 0) { if (auto p = _cache.NeighborLeft(_curr)) page = *p; x += pageW; }
        else if (x >= pageW) { if (auto p = _cache.NeighborRight(_curr)) page = *p; x -= pageW; }

        if (y < 0) { if (auto p = _cache.NeighborUp(_curr)) page = *p; y += pageH; }
        else if (y >= pageH) { if (auto p = _cache.NeighborDown(_curr)) page = *p; y -= pageH; }

        const int tx = std::clamp(static_cast<int>(std::floor(x / _ts)), 0, tSizeW - 1);
        const int ty = std::clamp(static_cast<int>(std::floor(y / _ts)), 0, tSizeH - 1);
        return _cache.GetTile(page, tx, ty);
    }
}