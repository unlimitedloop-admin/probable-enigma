//==============================================================================
// 
//  Project: mm2hack
//  BGTileMapProvider.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/systems/physics/ITileMapProvider.h"
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

    // Tile map provider implementation for BGTileManager
    class BGTileMapProvider : public ITileMapProvider
    {
    public:
        explicit BGTileMapProvider(const BGTileManager* mgr) noexcept;

        TileAttribute SampleTileAttribute(int tx, int ty) const override;
        int TileSize() const override;
        bool HasAdjacentRoomX(int dir) const override;
        bool HasAdjacentRoomY(int dir) const override;
        Vec2 MapPixelSize() const override;

    private:
        const std::wstring kClassName{ L"BGTileMapProvider" };

        const BGTileManager* _mgr{};
    };
}