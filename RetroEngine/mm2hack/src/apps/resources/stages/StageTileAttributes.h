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

    // HACK: This is a hardcoded tile attribute range for the demo stage 1. In a real implementation, this should be loaded from a configuration file or resource.
    constexpr TileAttributeRange STAGE1_TILEATTRIBUTES[] = {
        {  0,  31, TileAttribute::Empty },
        { 32,  95, TileAttribute::Solid },
        { 96,  96, TileAttribute::Ladder },
        {112, 112, TileAttribute::OneWayPlatform },
    };

    constexpr TileAttributeRange STAGE2_TILEATTRIBUTES[] = {
        {  0,  31, TileAttribute::Empty },
        { 32,  95, TileAttribute::Solid },
        { 96, 101, TileAttribute::Ladder },
        {112, 115, TileAttribute::OneWayPlatform },

        // Underwater attributes
        {128, 131, TileAttribute::Empty | TileAttribute::Water },
        {132, 135, TileAttribute::Ladder | TileAttribute::Water },
        {136, 143, TileAttribute::Solid | TileAttribute::Water },
        {144, 147, TileAttribute::Empty | TileAttribute::Water },
        {148, 151, TileAttribute::OneWayPlatform | TileAttribute::Water },
        {152, 159, TileAttribute::Solid | TileAttribute::Water },
    };
}