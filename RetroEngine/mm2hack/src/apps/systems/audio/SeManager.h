//==============================================================================
// 
//  Project: mm2hack
//  SeManager.h
// 
//  Contains the sound effect (SE) manager for audio playback.
// 
//==============================================================================
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "ChannelManager.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    enum class SePriority
    {
        Low,    // Low priority SE, can be interrupted by higher priority SE
        Normal, // Normal priority SE, will not be interrupted by lower priority SE
        High    // High priority SE, will interrupt any currently playing SE
    };

    // Sound Effect (SE) Manager
    class SeManager
    {
    public:
        explicit SeManager(ChannelManager& bgmChannels, int seChannelCount = 8);
        ~SeManager() = default;

        // Load SE data from file
        bool LoadSe(const std::wstring& name, const std::vector<std::wstring>& filepath, const std::vector<int>& volume, const std::vector<int>& targetChannels = {});
        // Play SE (search for an available channel, if none found, stop the oldest one and use it)
        void PlaySe(const std::wstring& name, int volume = -1);
        // Stop all SE
        void StopAll();
        // Pause all SE
        void Pause();
        // Resume all SE
        void Resume();
        // Update (check for SE end + restore BGM channel)
        void Update();
        // Volume control
        void SetMasterVolume(int volume);

    private:
        struct SeData
        {
            std::vector<std::wstring> filepaths;
            std::vector<int> volumes;
            std::vector<int> targetBgmChannels;
            SePriority priority = SePriority::Normal;
        };

        struct ActiveSeChannel
        {
            std::wstring seName;        // SE name being played
            int seChannelIndex = 0;     // SE channel index
        };

        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;

        ChannelManager _seChannels;                                 // SE channels manager
        ChannelManager& _bgmChannels;                               // BGM channels reference (for mute control)
        std::unordered_map<std::wstring, SeData> _seData;           // Name -> SE data
        std::unordered_map<int, ActiveSeChannel> _activeSeChannels; // Active SE channels (index -> SE name and channel index)
        std::vector<int> _bgmVolumeBackup;                          // Backup for restoring BGM channel volume after SE playback

        int _masterVolume = MAX_VOLUME;
    };
}