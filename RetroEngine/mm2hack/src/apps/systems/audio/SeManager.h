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

#include "ApuVoice.h"
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
            const std::vector<ApuVoice>& voices,
            const std::vector<SePriority> priority = {},
            double loopStart = 0.0,
            double loopEnd = 0.0
        );
        // Play SE (search for an available channel, if none found, stop the oldest one and use it)
        void PlaySe(const std::wstring& name, int volume = -1);
        // Stop all channels currently playing the named SE.
        void StopSe(const std::wstring& name);
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
            std::vector<ApuVoice> voices;           // Logical APU voices occupied by the SE stems
            std::vector<SePriority> priority;       // SE priorities
            double loopStart = 0.0;                 // Loop start in seconds
            double loopEnd = 0.0;                   // Loop end in seconds; disabled when <= loopStart
        };

        // Active SE channel information
        struct ActiveSeChannel
        {
            std::wstring seName;                    // SE name being played
            int seChannelIndex = 0;                 // SE channel index
        };

        bool canPlaySe_(const SeData& newSe) const; // Check if a new SE can be played based on priority
        void restoreBgmForSe_(const std::wstring& name);

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
