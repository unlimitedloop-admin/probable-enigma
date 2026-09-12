//==============================================================================
// 
//  Project: mm2hack
//  BGTileMapProvider.h
// 
//  Helper functions for retrieving BG tile attributes.
// 
//==============================================================================
#pragma once

#include "apps/systems/physics/ITileMapProvider.h"

#include <memory>
#include <string>

#include "apps/systems/physics/TileAttribute.h"

namespace mm2hack::apps::resources::bg
{
    class IMapPageSource;
}

namespace mm2hack::apps::rendering::bg
{
    using systems::physics::ITileMapProvider;
    using systems::physics::TileAttribute;

    class BGTileManager;

    // Tile map provider implementation backed by BGTileManager and a page source.
    class BGTileMapProvider final : public ITileMapProvider
    {
    public:
        BGTileMapProvider(
            const BGTileManager& manager,
            std::shared_ptr<const resources::bg::IMapPageSource> source);

        TileAttribute SampleTileAttributeOnPage(std::size_t pageIndex, int tx, int ty) const override;

    private:
        const std::wstring kClassName{ L"BGTileMapProvider" };

        const BGTileManager& _manager;
        std::shared_ptr<const resources::bg::IMapPageSource> _source;
    };
}
