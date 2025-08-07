//==============================================================================
// 
//  Project: mm2hack
//  GraphicsConfig.h
// 
//  Configuration for graphics settings.
// 
//==============================================================================
#pragma once

namespace mm2hack::config
{
    // Configuration for graphics settings
    struct GraphicsConfig
    {
        int resolutionIndex;    // Index of the selected resolution
        bool vsync;             // VSync enabled/disabled
        int fpsLimitIndex;      // FPS limit set by the user
    };
}