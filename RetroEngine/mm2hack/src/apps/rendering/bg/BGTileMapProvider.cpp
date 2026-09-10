#include "pch.h"

#include "BGTileMapProvider.h"

#include <stdexcept>

#include "apps/resources/bg/IMapPageSource.h"
#include "apps/systems/physics/TileAttribute.h"
#include "BGTileManager.h"

namespace mm2hack::apps::rendering::bg
{
    BGTileMapProvider::BGTileMapProvider(
        const BGTileManager& manager,
        std::shared_ptr<const resources::bg::IMapPageSource> source)
        : _manager(manager)
        , _source(std::move(source))
    {
        if (!_source)
        {
            throw std::invalid_argument("BGTileMapProvider requires a page source.");
        }
    }

    TileAttribute BGTileMapProvider::SampleTileAttributeOnPage(std::size_t pageIndex, int tx, int ty) const
    {
        const auto tile_id = _source->GetTile(pageIndex, tx, ty);
        return _manager.GetTileAttribute(tile_id);
    }
}
