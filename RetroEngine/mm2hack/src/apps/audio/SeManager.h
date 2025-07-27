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
        bool LoadSe(const std::wstring& name, const std::wstring& filepath);
        // Play SE (search for an available channel, if none found, stop the oldest one and use it)
        void PlaySe(const std::wstring& name, int volume = 255);
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
            int defaultVolume = 255;
        };

        ChannelManager _seChannels;                         // SE channels manager
        ChannelManager& _bgmChannels;                       // BGM channels reference (for mute control)
        std::unordered_map<std::wstring, SeData> _seData;   // Name -> SE data

        int _masterVolume = 255;
        std::vector<int> _bgmVolumeBackup;                  // Backup for restoring BGM channel volume
    };
}