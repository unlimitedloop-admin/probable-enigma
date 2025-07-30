#include "pch.h"

#include "AudioConfigLoader.h"

#include <nlohmann/json.hpp>
#include "utils/string_converter.h"

using json = nlohmann::json;

namespace mm2hack::apps::audio
{
    bool AudioConfigLoader::LoadFromFile(const std::wstring& filepath)
    {
        using namespace utils;

        _bgmConfigs.clear();
        _seConfigs.clear();

        std::string path_utf8 = wstring_to_utf8(filepath);

        std::ifstream ifs(path_utf8);
        if (!ifs.is_open()) return false;

        json j;
        ifs >> j;

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
                        chConfig.volume = ch.value("volume", 255);
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
                _bgmConfigs[utf8_to_wstring(name)] = bgmConfig;
            }
        }

        // SE
        if (j.contains("se"))
        {
            for (auto& [name, seJson] : j["se"].items())
            {
                SeConfig seConfig;
                seConfig.file = utf8_to_wstring(seJson.value("file", ""));
                seConfig.volume = seJson.value("volume", 255);
                if (seJson.contains("target_bgm_channels"))
                {
                    for (auto& ch : seJson["target_bgm_channels"])
                    {
                        if (ch.is_number_integer())
                        {
                            seConfig.targetBgmChannels.push_back(ch.get<int>());
                        }
                    }
                }
                _seConfigs[utf8_to_wstring(name)] = seConfig;
            }
        }

        return true;
    }
}