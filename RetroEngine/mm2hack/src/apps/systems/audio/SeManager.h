//==============================================================================
// 
//  Project: mm2hack
//  SeManager.h
// 
//  Contains the sound effect (SE) manager for audio playback.
// 
//==============================================================================
#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ApuVoice.h"
#include "ApuVoiceArbiter.h"
#include "ChannelManager.h"
#include "config/SystemConfig.h"
#include "ISoundChannel.h"
#include "SePriority.h"
#include "SeRestorePolicy.h"
#include "SeTransportState.h"

namespace mm2hack::apps::systems::audio
{
    class BgmManager;

    // Sound Effect (SE) Manager
    class SeManager
    {
    public:
        using SoundChannelFactory = std::function<std::unique_ptr<ISoundChannel>()>;

        SeManager();
        explicit SeManager(SoundChannelFactory channelFactory);
        ~SeManager() = default;

        // Load SE data from file
        bool LoadSe(
            const std::wstring& name,
            const std::vector<std::wstring>& filepath,
            const std::vector<int>& volume,
            const std::vector<ApuVoice>& voices,
            const std::vector<SePriority> priority = {},
            double loopStart = 0.0,
            double loopEnd = 0.0,
            SeRestorePolicy restorePolicy = SeRestorePolicy::Transient
        );
        // Play SE (search for an available channel, if none found, stop the oldest one and use it)
        void PlaySe(const std::wstring& name, int volume = -1);
        // Stop all channels currently playing the named SE.
        void StopSe(const std::wstring& name);
        // Stop all SE
        void StopAll();
        // Release playback resources and registered SE definitions.
        void Release();
        // Pause all SE
        void Pause();
        // Resume all SE
        void Resume();
        // Update (check for SE end + restore BGM channel)
        void Update();
        // Volume control
        void SetMasterVolume(int volume);

        // Capture, validate, and restore backend-independent continuous SE transport.
        bool CaptureState(SeTransportState& state) const;
        bool ValidateState(const SeTransportState& state) const;
        bool RestoreState(const SeTransportState& state);

        // Get the highest priority among currently playing SE
        SePriority GetCurrentMaxPriority() const;

        // Check if an SE currently owns a logical APU voice.
        bool IsVoiceOwnedBySe(ApuVoice voice) const;

        void SetBgmManager(BgmManager* manager) { _bgmManager = manager; }

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
            SeRestorePolicy restorePolicy = SeRestorePolicy::Transient;
        };

        void applyBgmOwnership_();
        void applyVoiceVolume_(ApuVoice voice);
        void stopPlaybackForOwners_(const std::vector<std::wstring>& owners);

    private:
        const std::wstring kClassName{ L"SeManager" };

        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;

        int _masterVolume = MAX_VOLUME;
        ChannelManager _seChannels;                                 // SE channels manager
        ApuVoiceArbiter _voiceArbiter;                              // Exclusive logical APU voice ownership
        std::unordered_map<std::wstring, SeData> _seData;           // Name -> SE data
        std::unordered_map<int, std::wstring> _channelToSeName;     // Channel index -> SE name mapping
        std::array<int, kApuVoiceCount> _logicalVolumes{};          // Per-voice volume before SE master volume
        std::array<bool, kApuVoiceCount> _pausedVoices{};           // Per-voice logical pause state

        BgmManager* _bgmManager = nullptr;                          // Pointer to the BGM manager for volume adjustments
    };
}
