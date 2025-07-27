//==============================================================================
// 
//  Project: mm2hack
//  AudioMixer.h
// 
//  Manages audio mixing for BGM and SE.
// 
//==============================================================================
#pragma once

#include "BgmManager.h"
#include "SeManager.h"

namespace mm2hack::apps::audio
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
        BgmManager& _bgm;
        SeManager& _se;

        int _masterVolume = 255;
        int _bgmVolume = 255;
        int _seVolume = 255;

        // Fade management
        bool _fading = false;
        int _fadeTarget = 255;
        int _fadeStep = 0;
        int _fadeFramesRemaining = 0;
    };
}
