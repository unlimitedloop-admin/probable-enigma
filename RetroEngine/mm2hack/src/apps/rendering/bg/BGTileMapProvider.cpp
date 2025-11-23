#include "pch.h"

#include "BGTileMapProvider.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/TileAttribute.h"
#include "BGTileManager.h"

namespace mm2hack::apps::rendering::bg
{
    using systems::physics::TileAttribute;

    BGTileMapProvider::BGTileMapProvider(const BGTileManager* mgr) noexcept
        : _mgr(mgr)
    {
    }

    TileAttribute BGTileMapProvider::SampleTileAttribute(int tx, int ty) const
    {
        if (!_mgr) return TileAttribute::Empty;

        // Out-of-bounds check, if out-of-bounds, return space-attr.
        if (tx < 0 || ty < 0 || tx >= _mgr->MapWidth() || ty >= _mgr->MapHeight())
            return TileAttribute::Empty;

        // Get tile attribute by tile coordinates.
        uint8_t id = _mgr->GetTile(tx, ty);
        return _mgr->GetTileAttribute(id);
    }

    int BGTileMapProvider::TileSize() const
    {
        return _mgr ? _mgr->TileSize() : 16;
    }

    bool BGTileMapProvider::HasAdjacentRoomX(int dir) const
    {
        // dir = -1 (左) / +1 (右)
        return _mgr ? _mgr->HasAdjacentRoomX(dir) : false;
    }

    bool BGTileMapProvider::HasAdjacentRoomY(int dir) const
    {
        return _mgr ? _mgr->HasAdjacentRoomY(dir) : false;
    }

    foundation::math::Vec2 BGTileMapProvider::MapPixelSize() const
    {
        if (!_mgr) return { 0,0 };
        return {
            static_cast<double>(_mgr->MapWidth() * _mgr->TileSize()),
            static_cast<double>(_mgr->MapHeight() * _mgr->TileSize())
        };
    }
}