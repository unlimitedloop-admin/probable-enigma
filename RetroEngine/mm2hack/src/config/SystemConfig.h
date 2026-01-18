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
    // This struct defines the system configuration settings for the mm2hack project.
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

        // NES palette configuration
        static const std::wstring kNESPaletteFilepath;              // Path to the NES palette file
        static constexpr int kNESPaletteSize = 64;                  // Size of the NES palette (number of colors)
        static constexpr int kDefaultNESPaletteIndex = 15U;         // Default NES palette index for background color (pitch black)
        static constexpr int kMakeSeqPaletteIndex = 0U;             // System palette index for the default color (gray)

        // Audio settings
        static constexpr int kAudioMaxVolume = 255;                 // Maximum audio volume

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

        static constexpr uint32_t kFeedbackOverlayDuration = 180;   // Duration for feedback overlay display (milliseconds)

        // Map data in binary format
        static constexpr size_t kMapBinaryUnitPageSize = 0x100;     // Size of one map binary unit page (bytes)
        static constexpr size_t kMapBinaryHeaderSize = 0x10;        // Size of the map binary header (bytes)


        // System parameters for physics and rendering
        static constexpr double kEpsilon = 0x00.01p0;               // Small epsilon value for calculations


        // NES Style properties
        // Tile Set
        static const unsigned int kTileSize = 16;                   // Tile size (pixels)
        static const unsigned int kTileSizeWidth = 16;              // BG Tile width (pixels)
        static const unsigned int kTileSizeHeight = 16;             // BG Tile height (pixels)
        static const unsigned int kTileCountX = 16;                 // Number of tiles in X direction
        static const unsigned int kTileCountY = 15;                 // Number of tiles in Y direction
    };
}