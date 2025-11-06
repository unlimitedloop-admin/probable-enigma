//==============================================================================
// 
//  Project: mm2hack
//  AudioConfigLoader.h
// 
//  Audio configuration loader that reads BGM and SE configurations from a JSON file.
// 
//==============================================================================
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    // Configuration structures for BGM and SE (Sound Effects)
    struct BgmChannelConfig
    {
        std::wstring file;
        int volume = config::SystemConfig::kAudioMaxVolume; // Volume level (0-255)
    };

    // Configuration structure for BGM, which includes multiple channels and loop points
    struct BgmConfig
    {
        std::vector<BgmChannelConfig> channels;
        double loopStart = 0.0;
        double loopEnd = 0.0;
    };

    struct SeChannelConfig
    {
        std::wstring file;
        int volume = config::SystemConfig::kAudioMaxVolume; // Volume level (0-255)
        int target_bgm_channels = -1; // Channel to restore BGM volume after SE playback, -1 means no specific channel
    };

    // Configuration structure for SE (Sound Effects), which includes the file and volume
    struct SeConfig
    {
        std::vector<SeChannelConfig> channels;  // SE channels configuration
    };

    // Audio configuration loader that reads BGM and SE configurations from a JSON file
    class AudioConfigLoader
    {
    public:
        // Loads audio configurations from a JSON file
        bool LoadFromFile(const std::wstring& filepath);

        const std::unordered_map<std::wstring, BgmConfig>& GetBgmConfigs() const { return _bgmConfigs; }
        const std::unordered_map<std::wstring, SeConfig>& GetSeConfigs() const { return _seConfigs; }

    private:
        const std::wstring kClassName = L"AudioConfigLoader";

        std::unordered_map<std::wstring, BgmConfig> _bgmConfigs;
        std::unordered_map<std::wstring, SeConfig> _seConfigs;
    };
}