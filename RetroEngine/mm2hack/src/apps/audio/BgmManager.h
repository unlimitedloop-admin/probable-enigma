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
#include <Windows.h>
#include "ChannelManager.h"

namespace mm2hack::apps::audio
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

        void Update();

        bool IsPlaying() const { return _isPlaying; }
        std::wstring GetCurrentBgmName() const { return _currentBgm; }

    private:
        struct BgmData
        {
            std::vector<std::wstring> filepaths;
            std::vector<int> volumes;
            double loopStart = 0.0;
            double loopEnd = 0.0;
        };

        ChannelManager& _channels;
        std::unordered_map<std::wstring, BgmData> _bgmData;

        std::wstring _currentBgm;
        bool _isPlaying = false;

        // Loop parameters
        double _loopStart = 0.0;
        double _loopEnd = 0.0;

        // Fade parameters
        bool _isFading = false;
        int _fadeTarget = 255;
        int _fadeStep = 0;
        int _fadeFramesRemaining = 0;
        int _masterVolume = 255;

        void ApplyFade();
        void CheckAndApplyLoop();
    };
}