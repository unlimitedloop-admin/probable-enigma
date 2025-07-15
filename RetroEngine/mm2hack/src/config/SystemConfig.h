//==============================================================================
// 
//  Project: mm2hack
//  SystemConfig.h
// 
//  Settings for the system configuration.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <string>

namespace mm2hack::config
{
    struct SystemConfig
    {
    public:
        SystemConfig() = delete;
        SystemConfig(const SystemConfig&) = delete;
        SystemConfig(SystemConfig&&) = delete;
        SystemConfig& operator=(const SystemConfig&) = delete;
        SystemConfig& operator=(SystemConfig&&) = delete;
        ~SystemConfig() = delete;
        // This struct is not copyable or movable (static member defined only)

        // The log file output destination
        static const std::wstring kLogFilePath;                     // Location of the log file
        static const std::wstring kLogFileName;                     // Name of the log file
        static const std::wstring kDxLibLogFileName;                // Name of the DxLib log file

        // NES palette file path
        static const std::wstring kNESPaletteFilepath;              // Path to the NES palette file
        static constexpr int kNESPaletteSize = 64;                  // Size of the NES palette (number of colors)
        static constexpr int kDefaultNESPaletteIndex = 15U;         // Default NES palette index for background color (pitch black)
        static constexpr int kMakeSeqPaletteIndex = 0U;             // System palette index for the default color (gray)

        // Frame rate settings
        static constexpr int kTargetFps = 60;                       // Frame rate (frames per second)

        // Window settings
        // NOTE: The default window size is 256x240 pixels and scaled to 512x480 pixels. 
        static const std::wstring kWindowClassName;                 // Window class name
        static constexpr int kScreenWidth = 256;                    // Width of the screen (pixels)
        static constexpr int kScreenHeight = 240;                   // Height of the screen (pixels)
        static constexpr int kScreenColorDepth = 16;                // Color depth (bits)
        static constexpr float kScreenScale = 2.0f;                 // Scale factor for the screen
        static constexpr float kScreenScaleMax = 4.0f;              // Maximum scale factor for the screen

        static constexpr uint32_t kCurrentSaveVersion = 1;          // Current save version
    };
}