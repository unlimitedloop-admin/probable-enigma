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
#include "ApuVoiceArbiter.h"
#include "ChannelManager.h"
#include "config/SystemConfig.h"
#include "SePriority.h"

namespace mm2hack::apps::systems::audio
{
    class AudioMixer;
    class BgmManager;

    // Sound Effect (SE) Manager
    class SeManager
    {
    public:
        explicit SeManager(ChannelManager& bgmChannels);
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

        void applyBgmOwnership_();
        void stopPlaybackForOwners_(const std::vector<std::wstring>& owners);

    private:
        const std::wstring kClassName{ L"SeManager" };

        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;

        int _masterVolume = MAX_VOLUME;
        ChannelManager _seChannels;                                 // SE channels manager
        ChannelManager& _bgmChannels;                               // BGM channels reference (for mute control)
        ApuVoiceArbiter _voiceArbiter;                              // Exclusive logical APU voice ownership
        std::unordered_map<std::wstring, SeData> _seData;           // Name -> SE data
        std::unordered_map<int, std::wstring> _channelToSeName;     // Channel index -> SE name mapping

        BgmManager* _bgmManager = nullptr;                          // Pointer to the BGM manager for volume adjustments
        AudioMixer* _mixer = nullptr;                               // Pointer to the audio mixer for volume control
    };
}
