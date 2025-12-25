//==============================================================================
// 
//  Project: mm2hack
//  HudConfig.h
// 
//  HUD (Heads-Up Display) configuration settings.
// 
//==============================================================================
#pragma once

namespace mm2hack::config
{
    // Configuration for the HUD (Heads-Up Display)
    struct HudConfig
    {
        bool showFps;           // Show FPS in the HUD
        bool showFrameTime;     // Show frame time in milliseconds
        bool showScrollLine;    // Show scroll line indicator
    };
}