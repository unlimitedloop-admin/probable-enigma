//==============================================================================
// 
//  Project: mm2hack
//  SaveData.h
// 
//  It's a structure that holds data for external data recording.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <vector>

namespace mm2hack::core::save
{
    // SaveData is a structure that holds data for external data recording
    struct SaveData
    {
        std::int32_t sequenceID = 0;
        std::int32_t sceneID = -1;
        std::vector<std::uint8_t> scenePayload;
    };
}
