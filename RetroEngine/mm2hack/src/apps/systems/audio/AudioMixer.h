//==============================================================================
// 
//  Project: mm2hack
//  AudioMixer.h
// 
//  Manages audio mixing for BGM and SE.
// 
//==============================================================================
#pragma once

#include <string>
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::audio
{
    class BgmManager;
    class SeManager;
}

namespace mm2hack::apps::systems::audio
{
    // AudioMixer is responsible for mixing BGM and SE volumes
    class AudioMixer
    {
    public:
        AudioMixer(BgmManager& bgm, SeManager& se)
            : _bgm(bgm), _se(se)
        {
        }

        // Set the master volume, BGM volume, and SE volume
        void SetMasterVolume(int volume);
        // Set the BGM volume and SE volume
        void SetBgmVolume(int volume);
        void SetSeVolume(int volume);
        void SetEnabled(bool enabled);

        // Get the current master volume, BGM volume, and SE volume
        int GetMasterVolume() const { return _masterVolume; }
        // Get the current BGM volume and SE volume
        int GetBgmVolume() const { return _bgmVolume; }
        int GetSeVolume() const { return _seVolume; }

        // Fade the master volume to a target value over a specified duration in frames
        void FadeMaster(int target, int durationFrames);
        // Every frame update for the mixer
        void Update();

    private:
        void applyVolumes_();   // Apply the calculated volumes to BGM and SE managers

    private:
        const std::wstring kClassName{ L"AudioMixer" };

        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;

        int _masterVolume = MAX_VOLUME;
        int _bgmVolume = MAX_VOLUME;
        int _seVolume = MAX_VOLUME;
        bool _enabled = true;

        // Fade management
        bool _fading = false;
        int _fadeTarget = MAX_VOLUME;
        int _fadeStep = 0;
        int _fadeFramesRemaining = 0;

        BgmManager& _bgm;       // Reference to BGM manager
        SeManager& _se;         // Reference to SE manager
    };
}