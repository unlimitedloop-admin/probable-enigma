//==============================================================================
// 
//  Project: mm2hack
//  ConfigUIManager.h
// 
//  Configuration of the application settings to be saved and loaded from an INI file.
// 
//==============================================================================
#pragma once

#include <string>
#include "config/HudConfig.h"

namespace mm2hack::config
{
    struct GraphicsConfig
    {
        int resolutionIndex;    // Index of the selected resolution
        bool vsync;             // VSync enabled/disabled
        int fpsLimitIndex;      // FPS limit set by the user
    };

    struct SoundConfig
    {
        int master = 80;        // Master volume level (0-100)
        int bgm = 80;           // Background music volume level (0-100)
        int se = 80;            // Sound effects volume level (0-100)
        bool enabled = true;    // Sound enabled/disabled
        int sourceIndex = 0;    // Index of the selected sound source (0 for default, 1 for add DPCM channel, etc.)
    };

    // ConfigUIManager is responsible for saving and loading the application settings to/from an INI file
    class ConfigUIManager final
    {
    public:
        // Save graphics configuration to the INI file
        static void SaveGraphicsConfig(const GraphicsConfig& config);
        // Load graphics configuration from the INI file
        static void LoadGraphicsConfig(GraphicsConfig& config);
        // Save sound configuration to the INI file
        static void SaveSoundConfig(const SoundConfig& config);
        // Load sound configuration from the INI file
        static void LoadSoundConfig(SoundConfig& config);
        // Save HUD configuration to the INI file
        static void SaveHudConfig(const HudConfig& config);
        // Load HUD configuration from the INI file
        static void LoadHudConfig(HudConfig& config);
        // Get the current HUD configuration from the cache
        static const HudConfig& GetCurrentHudConfig();
        // Set the current HUD configuration to the INI file and update the cache
        static void SetCurrentHudConfig(const HudConfig& config);

    private:
        static std::wstring GetIniPath();
        static HudConfig _cachedHudConfig;  // NOTE: Cached HUD configuration to avoid repeated file I/O.
    };
}