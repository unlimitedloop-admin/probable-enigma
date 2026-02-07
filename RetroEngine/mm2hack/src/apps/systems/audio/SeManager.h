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
#include "AudioConfigLoader.h"
#include "AudioMixer.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    // Sound Effect (SE) Manager
    class SeManager
    {
    public:
        explicit SeManager(ChannelManager& bgmChannels, int seChannelCount = 8);
        ~SeManager() = default;

        // Load SE data from file
        bool LoadSe(
            const std::wstring& name,
            const std::vector<std::wstring>& filepath,
            const std::vector<int>& volume,
            const std::vector<int>& targetChannels = {},
            const std::vector<SePriority> priority = {}
        );
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

        // Get the highest priority among currently playing SE
        SePriority GetCurrentMaxPriority() const;

        // Check if a specific BGM channel is muted due to SE playback
        bool IsBgmChannelMuted(int index) const;

        void SetBgmManager(BgmManager* manager) { _bgmManager = manager; }
        void SetAudioMixer(AudioMixer* mixer) { _mixer = mixer; }

    private:
        // SE data structure
        struct SeData
        {
            std::vector<std::wstring> filepaths;    // SE file paths
            std::vector<int> volumes;               // SE volumes
            std::vector<int> targetBgmChannels;     // Target BGM channels for muting
            std::vector<SePriority> priority;       // SE priorities
        };

        // Active SE channel information
        struct ActiveSeChannel
        {
            std::wstring seName;                    // SE name being played
            int seChannelIndex = 0;                 // SE channel index
        };

        bool canPlaySe_(const SeData& newSe) const; // Check if a new SE can be played based on priority

    private:
        const std::wstring kClassName{ L"SeManager" };

        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;

        int _masterVolume = MAX_VOLUME;
        ChannelManager _seChannels;                                 // SE channels manager
        ChannelManager& _bgmChannels;                               // BGM channels reference (for mute control)
        std::unordered_map<std::wstring, SeData> _seData;           // Name -> SE data
        std::unordered_map<int, ActiveSeChannel> _activeSeChannels; // Active SE channels (index -> SE name and channel index)
        std::unordered_map<int, std::wstring> _channelToSeName;     // Channel index -> SE name mapping
        std::vector<int> _bgmVolumeBackup;                          // Backup for restoring BGM channel volume after SE playback

        BgmManager* _bgmManager = nullptr;                          // Pointer to the BGM manager for volume adjustments
        AudioMixer* _mixer = nullptr;                               // Pointer to the audio mixer for volume control
    };
}