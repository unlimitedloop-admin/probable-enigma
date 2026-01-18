//==============================================================================
// 
//  Project: mm2hack
//  StageTileAttributes.h
// 
//  Configuration of stage tile attribute sets.
// 
//==============================================================================
#pragma once

#include "apps/systems/physics/TileAttribute.h"

namespace mm2hack::apps::resources::stages
{
    using systems::physics::TileAttribute;
    using systems::physics::TileAttributeRange;

    constexpr TileAttributeRange STAGE1_TILEATTRIBUTES[] = {
        {  0,  31, TileAttribute::Empty },
        { 32,  95, TileAttribute::Solid },
        { 96,  96, TileAttribute::Ladder },
        {112, 112, TileAttribute::OneWayPlatform },
    };
}