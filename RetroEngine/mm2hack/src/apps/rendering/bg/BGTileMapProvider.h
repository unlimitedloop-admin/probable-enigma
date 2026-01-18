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
#include "apps/resources/bg/IMapPageSource.h"
#include "apps/systems/physics/TileAttribute.h"
#include "BGTileManager.h"

namespace mm2hack::apps::foundation::math
{
    struct Vec2;
}

namespace mm2hack::apps::rendering::bg
{
    using foundation::math::Vec2;
    using systems::physics::ITileMapProvider;
    using systems::physics::TileAttribute;

    // Tile map provider implementation for BGTileManager, with optional page source
    class BGTileMapProvider : public ITileMapProvider
    {
    public:
        explicit BGTileMapProvider(const BGTileManager* mgr) noexcept;
        // New overload: provide a page source (shared ownership)
        explicit BGTileMapProvider(const BGTileManager* mgr, std::shared_ptr<apps::resources::bg::IMapPageSource> src) noexcept;

        TileAttribute SampleTileAttribute(int tx, int ty) const override;
        TileAttribute SampleTileAttributeOnPage(std::size_t pageIndex, int tx, int ty) const override;
        int TileSize() const override;
        bool HasAdjacentRoomX(int dir) const override;
        bool HasAdjacentRoomY(int dir) const override;
        Vec2 MapPixelSize() const override;

    private:
        const std::wstring kClassName{ L"BGTileMapProvider" };

        const BGTileManager* _mgr{};
        std::shared_ptr<apps::resources::bg::IMapPageSource> _src{}; // optional source for page-based queries
    };
}