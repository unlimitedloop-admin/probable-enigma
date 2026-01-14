//==============================================================================
// 
//  Project: mm2hack
//  ITileMapProvider.h
// 
//  These are functions to provide tile map information for physics calculations.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    using foundation::math::Vec2;

    // Interface for tile map providers
    class ITileMapProvider
    {
    public:
        virtual ~ITileMapProvider() = default;

        virtual TileAttribute SampleTileAttribute(int tx, int ty) const = 0;
        virtual TileAttribute SampleTileAttributeOnPage(std::size_t pageIndex, int tx, int ty) const = 0;
        virtual int TileSize() const = 0; // ex. 16
        virtual bool HasAdjacentRoomX(int dir /*-1:L,+1:R*/) const = 0;
        virtual bool HasAdjacentRoomY(int dir /*-1:U,+1:D*/) const = 0;
        virtual Vec2 MapPixelSize() const = 0;
    };
}