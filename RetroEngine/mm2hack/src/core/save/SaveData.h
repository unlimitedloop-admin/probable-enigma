//==============================================================================
// 
//  Project: mm2hack
//  SaveData.h
// 
//  It's a structure that holds data for external data recording.
// 
//==============================================================================
#pragma once

namespace mm2hack::core::save
{
    // SaveData is a structure that holds data for external data recording
    struct SaveData
    {
        int sequenceID = 0;
        int sceneID = 0;
        int phaseID = 0;

        // Add more field as needed of save data...
    };
}