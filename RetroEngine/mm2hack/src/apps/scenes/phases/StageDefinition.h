//==============================================================================
// 
//  Project: mm2hack
//  StageDefinition.h
// 
//  Properties container for stage definition data.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::scenes::phases
{
    // Definition of a stage property
    struct StageDefinition final
    {
        std::wstring map_binary_path{};
        int start_page_index{};
        foundation::math::Vec2 start_local_pos{};
    };
}