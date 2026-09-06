//==============================================================================
// 
//  Project: mm2hack
//  ITileMapProvider.h
// 
//  These are functions to provide tile map information for physics calculations.
// 
//==============================================================================
#pragma once

#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    // Interface for tile map providers
    class ITileMapProvider
    {
    public:
        virtual ~ITileMapProvider() = default;

        virtual TileAttribute SampleTileAttributeOnPage(std::size_t pageIndex, int tx, int ty) const = 0;
    };
}
