#include "pch.h"

#include "AudioInitializer.h"

#include "AudioConfigLoader.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "SeManager.h"

namespace mm2hack::apps::audio
{
    bool AudioInitializer::InitializeAudio(const std::wstring& configPath, BgmManager& bgmManager, SeManager& seManager, ChannelManager& bgmChannels)
    {
        AudioConfigLoader loader;
        if (!loader.LoadFromFile(configPath))
        {
            return false;
        }

        // Registration of BGM
        for (const auto& [name, config] : loader.GetBgmConfigs())
        {
            std::vector<std::wstring> filepaths;
            std::vector<int> volumes;
            for (const auto& ch : config.channels)
            {
                filepaths.push_back(ch.file);
                volumes.push_back(ch.volume);
            }
            bgmManager.RegisterBgm(name, filepaths, volumes, config.loopStart, config.loopEnd);

            // Initial volume settings
            for (size_t i = 0; i < config.channels.size(); ++i)
            {
                if (i < static_cast<size_t>(bgmChannels.GetChannelCount()))
                {
                    bgmChannels.SetVolume(static_cast<int>(i), config.channels[i].volume);
                }
            }
        }

        // Registration of SE
        for (const auto& [name, config] : loader.GetSeConfigs())
        {
            seManager.LoadSe(name, config.file, config.volume, config.targetBgmChannels);
        }

        return true;
    }
}