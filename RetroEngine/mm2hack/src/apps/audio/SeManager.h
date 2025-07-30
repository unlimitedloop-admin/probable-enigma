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

namespace mm2hack::apps::audio
{
    // Sound Effect (SE) Manager
    class SeManager
    {
    public:
        SeManager(ChannelManager& bgmChannels, int seChannelCount = 8);
        ~SeManager() = default;

        // Load SE data from file
        bool LoadSe(const std::wstring& name, const std::wstring& filepath, int volume = 255, const std::vector<int>& targetChannels = {});
        // Play SE (search for an available channel, if none found, stop the oldest one and use it)
        void PlaySe(const std::wstring& name, int volume = -1);
        // Stop all SE
        void StopAll();
        // Update (check for SE end + restore BGM channel)
        void Update();
        // Volume control
        void SetMasterVolume(int volume);

    private:
        struct SeData
        {
            std::wstring filepath;
            int volume = 255;
            std::vector<int> targetBgmChannels;     // Channels to restore BGM volume after SE playback
        };

        struct ActiveSeChannel
        {
            std::wstring seName;                    // SE name being played
            int seChannelIndex;                     // SE channel index
        };

        ChannelManager _seChannels;                                 // SE channels manager
        ChannelManager& _bgmChannels;                               // BGM channels reference (for mute control)
        std::unordered_map<std::wstring, SeData> _seData;           // Name -> SE data
        std::unordered_map<int, ActiveSeChannel> _activeSeChannels; // Active SE channels (index -> SE name and channel index)
        std::vector<int> _bgmVolumeBackup;                          // Backup for restoring BGM channel volume after SE playback

        int _masterVolume = 255;
    };
}