#include "pch.h"

#include "AudioConfigLoader.h"

#include <array>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "ApuVoice.h"
#include "utils/string_converter.h"

using json = nlohmann::json;

namespace mm2hack::apps::systems::audio
{
    namespace
    {
        bool try_read_voice(const json& channel_json, ApuVoice& voice)
        {
            if (!channel_json.contains("voice") || !channel_json["voice"].is_string())
            {
                return false;
            }
            return TryParseApuVoice(channel_json["voice"].get<std::string>(), voice);
        }

        template<typename ChannelConfig>
        bool has_unique_voices(const std::vector<ChannelConfig>& channels)
        {
            std::array<bool, kApuVoiceCount> occupied{};
            for (const auto& channel : channels)
            {
                const std::size_t index = ToIndex(channel.voice);
                if (index >= occupied.size() || occupied[index])
                {
                    return false;
                }
                occupied[index] = true;
            }
            return true;
        }
    }

    bool AudioConfigLoader::LoadFromFile(const std::wstring& filepath)
    {
        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;
        using namespace utils;

        std::unordered_map<std::wstring, BgmConfig> bgm_configs;
        std::unordered_map<std::wstring, SeConfig> se_configs;
        json j;
        try
        {
            std::ifstream ifs(wstring_to_utf8(filepath));
            if (!ifs.is_open()) return false;
            ifs >> j;
        }
        catch (const json::exception&)
        {
            return false;
        }

        // BGM
        if (j.contains("bgm"))
        {
            for (auto& [name, bgmJson] : j["bgm"].items())
            {
                BgmConfig bgmConfig;
                if (bgmJson.contains("channels"))
                {
                    for (auto& ch : bgmJson["channels"])
                    {
                        BgmChannelConfig chConfig;
                        chConfig.file = utf8_to_wstring(ch.value("file", ""));
                        chConfig.volume = ch.value("volume", MAX_VOLUME);
                        if (chConfig.file.empty() || !try_read_voice(ch, chConfig.voice)) return false;
                        bgmConfig.channels.push_back(chConfig);
                    }
                }
                if (bgmJson.contains("loop_start"))
                {
                    bgmConfig.loopStart = bgmJson.value("loop_start", 0.0);
                }
                if (bgmJson.contains("loop_end"))
                {
                    bgmConfig.loopEnd = bgmJson.value("loop_end", 0.0);
                }
                if (bgmConfig.channels.empty() || !has_unique_voices(bgmConfig.channels)) return false;
                bgm_configs[utf8_to_wstring(name)] = std::move(bgmConfig);
            }
        }

        // SE
        if (j.contains("se"))
        {
            for (auto& [name, seJson] : j["se"].items())
            {
                SeConfig seConfig;
                // If the "channels" array exists, import each channel.
                if (seJson.contains("channels") && seJson["channels"].is_array())
                {
                    for (auto& ch : seJson["channels"])
                    {
                        SeChannelConfig chConfig;
                        chConfig.file = utf8_to_wstring(ch.value("file", ""));
                        chConfig.volume = ch.value("volume", MAX_VOLUME);
                        if (chConfig.file.empty() || !try_read_voice(ch, chConfig.voice)) return false;
                        int chPriority = ch.value("priority", 1);
                        chConfig.priority =
                            chPriority == 2 ? SePriority::High :
                            chPriority == 1 ? SePriority::Normal :
                            SePriority::Low;
                        seConfig.channels.push_back(chConfig);
                    }
                }
                // Older: If the "file" and "volume" keys exist, import them as a single channel.
                else
                {
                    SeChannelConfig chConfig;
                    chConfig.file = utf8_to_wstring(seJson.value("file", ""));
                    chConfig.volume = seJson.value("volume", MAX_VOLUME);
                    if (chConfig.file.empty() || !try_read_voice(seJson, chConfig.voice)) return false;
                    int seJsonPriority = seJson.value("priority", 1);
                    chConfig.priority =
                        seJsonPriority == 2 ? SePriority::High :
                        seJsonPriority == 1 ? SePriority::Normal :
                        SePriority::Low;
                    seConfig.channels.push_back(chConfig);
                }
                seConfig.loopStart = seJson.value("loop_start", 0.0);
                seConfig.loopEnd = seJson.value("loop_end", 0.0);
                if (seConfig.channels.empty() || !has_unique_voices(seConfig.channels)) return false;
                se_configs[utf8_to_wstring(name)] = std::move(seConfig);
            }
        }

        _bgmConfigs = std::move(bgm_configs);
        _seConfigs = std::move(se_configs);
        return true;
    }
}
