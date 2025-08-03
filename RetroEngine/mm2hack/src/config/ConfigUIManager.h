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
        static void SaveGraphicsConfig(const GraphicsConfig& config);
        static void LoadGraphicsConfig(GraphicsConfig& config);
        static void SaveSoundConfig(const SoundConfig& config);
        static void LoadSoundConfig(SoundConfig& config);

    private:
        static std::wstring GetIniPath();
    };
}