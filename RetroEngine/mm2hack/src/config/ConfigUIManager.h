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
#include "config/GraphicsConfig.h"
#include "config/HudConfig.h"
#include "config/SoundConfig.h"

namespace mm2hack::config
{
    // ConfigUIManager is responsible for saving and loading the application settings to/from an INI file
    class ConfigUIManager final
    {
    public:
        ConfigUIManager() = delete;
        ~ConfigUIManager() = delete;
        ConfigUIManager(const ConfigUIManager&) = delete;
        ConfigUIManager& operator=(const ConfigUIManager&) = delete;
        ConfigUIManager(ConfigUIManager&&) = delete;
        ConfigUIManager& operator=(ConfigUIManager&&) = delete;
        // This class is not copyable or movable (static class)

        // Save/Load graphics configuration to/from the INI file
        static void SaveGraphicsConfig(const GraphicsConfig& config);
        static void LoadGraphicsConfig(GraphicsConfig& config);

        // Save/Load sound configuration to/from the INI file
        static void SaveSoundConfig(const SoundConfig& config);
        static void LoadSoundConfig(SoundConfig& config);

        // Save/Load HUD configuration to/from the INI file
        static void SaveHudConfig(const HudConfig& config);
        static void LoadHudConfig(HudConfig& config);
        // Get the current HUD configuration from the cache
        static const HudConfig& GetCurrentHudConfig();
        // Set the current HUD configuration to the INI file and update the cache
        static void SetCurrentHudConfig(const HudConfig& config);

    private:
        static HudConfig _cachedHudConfig;  // Cached HUD configuration to avoid repeated file I/O.

        static std::wstring GetIniPath();
    };
}