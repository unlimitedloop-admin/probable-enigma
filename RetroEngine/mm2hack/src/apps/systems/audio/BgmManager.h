//==============================================================================
// 
//  Project: mm2hack
//  BgmManager.h
// 
//  Music manager for background music (BGM).
// 
//==============================================================================
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "ApuVoice.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    class ChannelManager;
    class SeManager;

    // BgmManager is responsible for managing background music (BGM) playback, including registration, playback, stopping, and fading
    class BgmManager
    {
    public:
        explicit BgmManager(ChannelManager& channels);
        ~BgmManager() = default;

        // Register a BGM with its name, file paths, volumes, and optional loop points
        bool RegisterBgm(const std::wstring& name, const std::vector<std::wstring>& filepaths,
            const std::vector<int>& volumes, const std::vector<ApuVoice>& voices,
            double loopStart = 0.0, double loopEnd = 0.0);
        // Play a registered BGM by name
        bool Play(const std::wstring& name);
        // Stop the currently playing BGM
        void Stop();
        // Release playback resources and registered BGM definitions.
        void Release();
        // Pause the currently playing BGM
        void Pause();
        // Resume the paused BGM
        void Resume();

        // Fade out the currently playing BGM over a specified number of frames
        void FadeOut(int durationFrames);
        // Fade in the currently playing BGM over a specified number of frames
        void FadeIn(int durationFrames);

        // Set the master volume for all BGM channels (0-255)
        void SetMasterVolume(int volume);
        // Get the current master volume (0-255)
        int GetMasterVolume() const { return _masterVolume; }
        // Reapply logical BGM volumes after APU voice ownership changes.
        void RefreshOutputVolumes();

        // Update the BGM manager (handle fading and looping)
        void Update();
        // Check if a BGM is currently playing
        bool IsPlaying() const { return _isPlaying; }
        // Get the name of the currently playing BGM
        std::wstring GetCurrentBgmName() const { return _currentBgm; }

        void SetSeManager(SeManager* manager) { _seManager = manager; }

    private:
        void applyFade_();           // Apply fade effect if active
        void checkAndApplyLoop_();   // Check and apply loop points if necessary
        int effectiveVolume_(ApuVoice voice, int logicalVolume) const;

    private:
        struct BgmData
        {
            std::vector<std::wstring> filepaths;
            std::vector<int> volumes;
            std::vector<ApuVoice> voices;
            double loopStart = 0.0;
            double loopEnd = 0.0;
        };

        const std::wstring kClassName{ L"BgmManager" };

        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;
        int _masterVolume = MAX_VOLUME;

        ChannelManager& _channels;                          // Reference to the channel manager for audio playback
        std::unordered_map<std::wstring, BgmData> _bgmData; // Registered BGM data

        std::wstring _currentBgm;                           // Name of the currently playing BGM
        std::vector<int> _logical_volumes;                  // Logical volumes before SE voice preemption
        bool _isPlaying = false;

        // Loop parameters
        double _loopStart = 0.0;
        double _loopEnd = 0.0;

        // Fade parameters
        bool _isFading = false;
        int _fadeTarget = MAX_VOLUME;
        int _fadeStep = 0;
        int _fadeFramesRemaining = 0;

        SeManager* _seManager = nullptr;                    // Pointer to the SE manager for sound effect interactions
    };
}
