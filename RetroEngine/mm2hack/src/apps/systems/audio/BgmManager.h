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
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    class ChannelManager;
}

namespace mm2hack::apps::systems::audio
{
    // BgmManager is responsible for managing background music (BGM) playback, including registration, playback, stopping, and fading
    class BgmManager
    {
    public:
        explicit BgmManager(ChannelManager& channels);
        ~BgmManager() = default;

        // Register a BGM with its name, file paths, volumes, and optional loop points
        bool RegisterBgm(const std::wstring& name, const std::vector<std::wstring>& filepaths,
            const std::vector<int>& volumes, double loopStart = 0.0, double loopEnd = 0.0);
        // Play a registered BGM by name
        bool Play(const std::wstring& name);
        // Stop the currently playing BGM
        void Stop();
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

        // Update the BGM manager (handle fading and looping)
        void Update();
        // Check if a BGM is currently playing
        bool IsPlaying() const { return _isPlaying; }
        // Get the name of the currently playing BGM
        std::wstring GetCurrentBgmName() const { return _currentBgm; }

    private:
        void applyFade_();           // Apply fade effect if active
        void checkAndApplyLoop_();   // Check and apply loop points if necessary

    private:
        struct BgmData
        {
            std::vector<std::wstring> filepaths;
            std::vector<int> volumes;
            double loopStart = 0.0;
            double loopEnd = 0.0;
        };

        const std::wstring kClassName{ L"BgmManager" };

        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;

        ChannelManager& _channels;                          // Reference to the channel manager for audio playback
        std::unordered_map<std::wstring, BgmData> _bgmData; // Registered BGM data

        std::wstring _currentBgm;                           // Name of the currently playing BGM
        bool _isPlaying = false;

        // Loop parameters
        double _loopStart = 0.0;
        double _loopEnd = 0.0;

        // Fade parameters
        bool _isFading = false;
        int _fadeTarget = MAX_VOLUME;
        int _fadeStep = 0;
        int _fadeFramesRemaining = 0;
        int _masterVolume = MAX_VOLUME;
    };
}