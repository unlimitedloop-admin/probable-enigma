#include "pch.h"

#include "AudioInitializer.h"

#include <string>
#include <vector>

#include "ApuVoice.h"
#include "AudioConfigLoader.h"
#include "BgmManager.h"
#include "SeManager.h"
#include "SePriority.h"

namespace mm2hack::apps::systems::audio
{
    bool AudioInitializer::InitializeAudio(
        const std::wstring& configPath,
        BgmManager& bgmManager,
        SeManager& seManager)
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
            if (!bgmManager.RegisterBgm(
                name, filepaths, volumes, voices, config.loopStart, config.loopEnd))
            {
                return false;
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
            if (!seManager.LoadSe(
                name,
                filepaths,
                volumes,
                voices,
                priorities,
                config.loopStart,
                config.loopEnd))
            {
                return false;
            }
        }

        return true;
    }
}
