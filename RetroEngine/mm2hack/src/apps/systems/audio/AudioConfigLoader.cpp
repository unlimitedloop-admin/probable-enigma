#include "pch.h"

#include "AudioConfigLoader.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "ApuVoice.h"
#include "config/SystemConfig.h"
#include "SePriority.h"
#include "utils/string_converter.h"

using json = nlohmann::json;

namespace mm2hack::apps::systems::audio
{
    namespace
    {
        bool try_read_voice(const json& channel_json, ApuVoice& voice)
        {
            const auto value = channel_json.find("voice");
            if (value == channel_json.end() || !value->is_string())
            {
                return false;
            }
            return TryParseApuVoice(value->get<std::string>(), voice);
        }

        bool try_read_integer(
            const json& object,
            const char* key,
            int default_value,
            int minimum,
            int maximum,
            int& result)
        {
            const auto value = object.find(key);
            if (value == object.end())
            {
                result = default_value;
                return true;
            }
            if (!value->is_number_integer()) return false;

            const std::int64_t parsed = value->get<std::int64_t>();
            if (parsed < minimum || parsed > maximum) return false;
            result = static_cast<int>(parsed);
            return true;
        }

        bool try_read_nonnegative_number(
            const json& object,
            const char* key,
            double& result)
        {
            const auto value = object.find(key);
            if (value == object.end())
            {
                result = 0.0;
                return true;
            }
            if (!value->is_number()) return false;

            const double parsed = value->get<double>();
            if (!std::isfinite(parsed) || parsed < 0.0) return false;
            result = parsed;
            return true;
        }

        bool try_read_file(const json& channel_json, std::wstring& file)
        {
            const auto value = channel_json.find("file");
            if (value == channel_json.end() || !value->is_string())
            {
                return false;
            }
            file = utils::utf8_to_wstring(value->get<std::string>());
            return !file.empty();
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

        bool try_parse_bgm_channel(const json& source, BgmChannelConfig& result)
        {
            if (!source.is_object() ||
                !try_read_file(source, result.file) ||
                !try_read_voice(source, result.voice))
            {
                return false;
            }
            return try_read_integer(
                source,
                "volume",
                config::SystemConfig::kAudioMaxVolume,
                0,
                config::SystemConfig::kAudioMaxVolume,
                result.volume);
        }

        bool try_parse_se_channel(const json& source, SeChannelConfig& result)
        {
            int priority = static_cast<int>(SePriority::Normal);
            if (!source.is_object() ||
                !try_read_file(source, result.file) ||
                !try_read_voice(source, result.voice) ||
                !try_read_integer(
                    source,
                    "volume",
                    config::SystemConfig::kAudioMaxVolume,
                    0,
                    config::SystemConfig::kAudioMaxVolume,
                    result.volume) ||
                !try_read_integer(
                    source,
                    "priority",
                    static_cast<int>(SePriority::Normal),
                    static_cast<int>(SePriority::Low),
                    static_cast<int>(SePriority::High),
                    priority))
            {
                return false;
            }
            result.priority = static_cast<SePriority>(priority);
            return true;
        }

        bool try_parse_bgm(const json& source, BgmConfig& result)
        {
            if (!source.is_object()) return false;
            const auto channels = source.find("channels");
            if (channels == source.end() || !channels->is_array() || channels->empty())
            {
                return false;
            }

            for (const auto& channel_json : *channels)
            {
                BgmChannelConfig channel{};
                if (!try_parse_bgm_channel(channel_json, channel)) return false;
                result.channels.push_back(std::move(channel));
            }
            return has_unique_voices(result.channels) &&
                try_read_nonnegative_number(source, "loop_start", result.loopStart) &&
                try_read_nonnegative_number(source, "loop_end", result.loopEnd);
        }

        bool try_parse_se(const json& source, SeConfig& result)
        {
            if (!source.is_object()) return false;
            const auto channels = source.find("channels");
            if (channels != source.end())
            {
                if (!channels->is_array() || channels->empty()) return false;
                for (const auto& channel_json : *channels)
                {
                    SeChannelConfig channel{};
                    if (!try_parse_se_channel(channel_json, channel)) return false;
                    result.channels.push_back(std::move(channel));
                }
            }
            else
            {
                SeChannelConfig channel{};
                if (!try_parse_se_channel(source, channel)) return false;
                result.channels.push_back(std::move(channel));
            }

            return has_unique_voices(result.channels) &&
                try_read_nonnegative_number(source, "loop_start", result.loopStart) &&
                try_read_nonnegative_number(source, "loop_end", result.loopEnd);
        }

        bool try_parse_config(
            const json& source,
            std::unordered_map<std::wstring, BgmConfig>& bgm_configs,
            std::unordered_map<std::wstring, SeConfig>& se_configs)
        {
            if (!source.is_object()) return false;

            const auto bgm = source.find("bgm");
            if (bgm != source.end())
            {
                if (!bgm->is_object()) return false;
                for (const auto& [name, bgm_json] : bgm->items())
                {
                    BgmConfig config{};
                    if (name.empty() || !try_parse_bgm(bgm_json, config)) return false;
                    bgm_configs.emplace(utils::utf8_to_wstring(name), std::move(config));
                }
            }

            const auto se = source.find("se");
            if (se != source.end())
            {
                if (!se->is_object()) return false;
                for (const auto& [name, se_json] : se->items())
                {
                    SeConfig config{};
                    if (name.empty() || !try_parse_se(se_json, config)) return false;
                    se_configs.emplace(utils::utf8_to_wstring(name), std::move(config));
                }
            }
            return true;
        }
    }

    bool AudioConfigLoader::LoadFromFile(const std::wstring& filepath)
    {
        try
        {
            std::ifstream stream(utils::wstring_to_utf8(filepath), std::ios::binary);
            if (!stream.is_open()) return false;

            const std::string source{
                std::istreambuf_iterator<char>(stream),
                std::istreambuf_iterator<char>()
            };
            if (stream.bad()) return false;
            return LoadFromJson(source);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool AudioConfigLoader::LoadFromJson(std::string_view source)
    {
        try
        {
            const json document = json::parse(source.begin(), source.end());
            std::unordered_map<std::wstring, BgmConfig> bgm_configs;
            std::unordered_map<std::wstring, SeConfig> se_configs;
            if (!try_parse_config(document, bgm_configs, se_configs)) return false;

            _bgmConfigs = std::move(bgm_configs);
            _seConfigs = std::move(se_configs);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
}
