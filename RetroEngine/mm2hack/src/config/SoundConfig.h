//==============================================================================
// 
//  Project: mm2hack
//  SoundConfig.h
// 
//  Configuration for sound settings.
// 
//==============================================================================
#pragma once

namespace mm2hack::config
{
    struct SoundConfig
    {
        int master = 80;        // Master volume level (0-100)
        int bgm = 80;           // Background music volume level (0-100)
        int se = 80;            // Sound effects volume level (0-100)
        bool enabled = true;    // Sound enabled/disabled
        int sourceIndex = 0;    // Index of the selected sound source (0 for default, 1 for add DPCM channel, etc.)
    };
}