#include "pch.h"

#include "BGTileMapProvider.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/resources/bg/IMapPageSource.h"
#include "apps/systems/physics/TileAttribute.h"
#include "BGTileManager.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::rendering::bg
{
    using systems::physics::TileAttribute;
    using apps::resources::bg::IMapPageSource;

    BGTileMapProvider::BGTileMapProvider(const BGTileManager* mgr) noexcept
        : _mgr(mgr)
    {
    }

    BGTileMapProvider::BGTileMapProvider(const BGTileManager* mgr, std::shared_ptr<IMapPageSource> src) noexcept
        : _mgr(mgr), _src(std::move(src))
    {
    }

    TileAttribute BGTileMapProvider::SampleTileAttribute(int tx, int ty) const
    {
        // If a page source is provided, use it first.
        if (_src)
        {
            // Convert global tile coords to page-local coords.
            const int pageTileW = config::SystemConfig::kTileCountX;
            const int pageTileH = config::SystemConfig::kTileCountY;

            const int totalTileW = (_src->MapWidth() > 0) ? _src->MapWidth() : pageTileW;
            const int pagesPerRow = std::max(1, totalTileW / pageTileW);

            const int pageX = tx / pageTileW;
            const int pageY = ty / pageTileH;
            const std::size_t pageIndex = static_cast<std::size_t>(pageY * pagesPerRow + pageX);

            const int localTx = tx % pageTileW;
            const int localTy = ty % pageTileH;

            const std::uint8_t id = _src->GetTile(pageIndex, localTx, localTy);
            return _mgr ? _mgr->GetTileAttribute(id) : TileAttribute::Empty;
        }

        // Fallback: use BGTileManager directly (existing behavior)
        if (!_mgr) return TileAttribute::Empty;
        if (tx < 0 || ty < 0 || tx >= _mgr->MapWidth() || ty >= _mgr->MapHeight())
            return TileAttribute::Empty;

        uint8_t id = _mgr->GetTile(tx, ty);
        return _mgr->GetTileAttribute(id);
    }

    TileAttribute BGTileMapProvider::SampleTileAttributeOnPage(std::size_t pageIndex, int tx, int ty) const
    {
        const auto tileId = _src->GetTile(pageIndex, tx, ty);
        return _mgr->GetTileAttribute(tileId);
    }

    int BGTileMapProvider::TileSize() const
    {
        if (_src) return _src->TileSize();
        return _mgr ? _mgr->TileSize() : 16;
    }

    bool BGTileMapProvider::HasAdjacentRoomX(int dir) const
    {
        if (!_src || !_mgr) return false;
        // dir = -1 (L) / +1 (R)
        // Query neighbor from page source: find any page with adjacency in requested dir.
        // Simplified: return whether pages exist horizontally in overall map.
        const int pageTileW = config::SystemConfig::kTileCountX;
        const int totalW = _src->MapWidth();
        const int pagesPerRow = std::max(1, totalW / pageTileW);
        return pagesPerRow > 1;
    }

    bool BGTileMapProvider::HasAdjacentRoomY(int dir) const
    {
        if (!_src || !_mgr) return false;
        // dir = -1 (U) / +1 (D)
        // Query neighbor from page source: find any page with adjacency in requested dir.
        // Simplified: return whether pages exist vertically in overall map.
        const int pageTileH = config::SystemConfig::kTileCountY;
        const int totalH = _src->MapHeight();
        const int rows = std::max(1, totalH / pageTileH);
        return rows > 1;
    }

    foundation::math::Vec2 BGTileMapProvider::MapPixelSize() const
    {
        if (_src)
        {
            const int tw = _src->MapWidth();
            const int th = _src->MapHeight();
            return { static_cast<double>(tw * (_mgr ? _mgr->TileSize() : config::SystemConfig::kTileSize)),
                     static_cast<double>(th * (_mgr ? _mgr->TileSize() : config::SystemConfig::kTileSize)) };
        }
        if (!_mgr) return { 0,0 };
        return {
            static_cast<double>(_mgr->MapWidth() * _mgr->TileSize()),
            static_cast<double>(_mgr->MapHeight() * _mgr->TileSize())
        };
    }
}