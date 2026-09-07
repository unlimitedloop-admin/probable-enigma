#include "pch.h"

#include "AudioInitializer.h"

#include <cstddef>
#include <string>
#include <vector>

#include "ApuVoice.h"
#include "AudioConfigLoader.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "SeManager.h"

namespace mm2hack::apps::systems::audio
{
    bool AudioInitializer::InitializeAudio(const std::wstring& configPath, BgmManager& bgmManager, SeManager& seManager, ChannelManager& bgmChannels, ChannelManager& seChannels)
    {
        AudioConfigLoader loader;
        if (!loader.LoadFromFile(configPath))
        {
            return false;
        }

        // Registration of BGM.
        for (const auto& [name, config] : loader.GetBgmConfigs())
        {
            std::vector<std::wstring> filepaths;
            std::vector<int> volumes;
            std::vector<ApuVoice> voices;
            for (const auto& ch : config.channels)
            {
                filepaths.push_back(ch.file);
                volumes.push_back(ch.volume);
                voices.push_back(ch.voice);
            }
            bgmManager.RegisterBgm(name, filepaths, volumes, voices, config.loopStart, config.loopEnd);

            // Initial volume settings.
            for (size_t i = 0; i < config.channels.size(); ++i)
            {
                const int channel_index = static_cast<int>(ToIndex(config.channels[i].voice));
                if (channel_index < bgmChannels.GetChannelCount())
                {
                    bgmChannels.SetVolume(channel_index, config.channels[i].volume);
                }
            }
        }

        // Registration of SE.
        for (const auto& [name, config] : loader.GetSeConfigs())
        {
            std::vector<std::wstring> filepaths;
            std::vector<int> volumes;
            std::vector<ApuVoice> voices;
            std::vector<SePriority> priorities;
            for (const auto& ch : config.channels)
            {
                filepaths.push_back(ch.file);
                volumes.push_back(ch.volume);
                voices.push_back(ch.voice);
                priorities.push_back(ch.priority);

            }
            seManager.LoadSe(
                name,
                filepaths,
                volumes,
                voices,
                priorities,
                config.loopStart,
                config.loopEnd);

            // Initial volume settings.
            for (size_t i = 0; i < config.channels.size(); ++i)
            {
                if (i < static_cast<size_t>(seChannels.GetChannelCount()))
                {
                    seChannels.SetVolume(static_cast<int>(i), config.channels[i].volume);
                }
            }
        }

        return true;
    }
}
