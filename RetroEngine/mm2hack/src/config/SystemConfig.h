//==============================================================================
// 
//  Project: mm2hack
//  SystemConfig.h
// 
//  Settings for the system configuration.
// 
//==============================================================================
#pragma once

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
        
        // Frame rate settings
        static constexpr int kTargetFps = 60;                       // Frame rate (frames per second)
    };
}