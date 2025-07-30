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

namespace mm2hack::apps::audio
{
    // Configuration structures for BGM and SE (Sound Effects)
    struct BgmChannelConfig
    {
        std::wstring file;
        int volume = 255;
    };

    // Configuration structure for BGM, which includes multiple channels and loop points
    struct BgmConfig
    {
        std::vector<BgmChannelConfig> channels;
        double loopStart = 0.0;
        double loopEnd = 0.0;
    };

    // Configuration structure for SE (Sound Effects), which includes the file and volume
    struct SeConfig
    {
        std::wstring file;
        int volume = 255;
        std::vector<int> targetBgmChannels;     // Channels to restore BGM volume after SE playback
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
        std::unordered_map<std::wstring, BgmConfig> _bgmConfigs;
        std::unordered_map<std::wstring, SeConfig> _seConfigs;
    };
}