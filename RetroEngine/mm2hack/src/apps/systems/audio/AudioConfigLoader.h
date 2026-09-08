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
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ApuVoice.h"
#include "config/SystemConfig.h"
#include "SePriority.h"

namespace mm2hack::apps::systems::audio
{
    // Configuration structures for BGM and SE (Sound Effects)
    struct BgmChannelConfig
    {
        std::wstring file;
        int volume = config::SystemConfig::kAudioMaxVolume; // Volume level (0-255)
        ApuVoice voice = ApuVoice::Pulse1;
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
        ApuVoice voice = ApuVoice::Pulse1;                  // Logical APU voice occupied by the SE stem
        SePriority priority = SePriority::Normal;           // Priority of the SE
    };

    // Configuration structure for SE (Sound Effects), which includes the file and volume
    struct SeConfig
    {
        std::vector<SeChannelConfig> channels;  // SE channels configuration
        double loopStart = 0.0;
        double loopEnd = 0.0;
    };

    // Audio configuration loader that reads BGM and SE configurations from a JSON file
    class AudioConfigLoader
    {
    public:
        // Loads audio configurations from a JSON file
        bool LoadFromFile(const std::wstring& filepath);
        // Parses audio configurations from UTF-8 JSON without accessing audio or files.
        bool LoadFromJson(std::string_view source);

        const std::unordered_map<std::wstring, BgmConfig>& GetBgmConfigs() const { return _bgmConfigs; }
        const std::unordered_map<std::wstring, SeConfig>& GetSeConfigs() const { return _seConfigs; }

    private:
        const std::wstring kClassName{ L"AudioConfigLoader" };

        std::unordered_map<std::wstring, BgmConfig> _bgmConfigs;
        std::unordered_map<std::wstring, SeConfig> _seConfigs;
    };
}
