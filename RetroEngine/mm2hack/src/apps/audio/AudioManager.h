//==============================================================================
// 
//  Project: mm2hack
//  AudioManager.h
// 
//  A class that manages the execution control of all sound sources.
// 
//==============================================================================
#pragma once

#include <string>
#include "AudioConfigLoader.h"
#include "AudioMixer.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "SeManager.h"

namespace mm2hack::apps::audio
{
    // AudioManager is the main class that manages BGM and SE playback
    class AudioManager
    {
    public:
        AudioManager();
        ~AudioManager() = default;

        bool Initialize(const std::wstring& configPath);

        // Controlling BGM
        void PlayBgm(const std::wstring& name);
        void StopBgm();
        void FadeOutBgm(int frames);
        void SetBgmVolume(int volume);

        // Controlling SE
        void PlaySe(const std::wstring& name);
        void SetSeVolume(int volume);

        // Master volume leveling
        void SetMasterVolume(int volume);

        // Pause / resume BGM and SE
        void Pause();
        void Resume();

        // Enable / disable all sounds
        void SetEnabled(bool enabled);
        bool IsEnabled() const { return _enabled; }

        // Every updates
        void Update();

        void Release();

    private:
        ChannelManager _bgmChannels;
        ChannelManager _seChannels;
        BgmManager _bgmManager;
        SeManager _seManager;
        AudioMixer _mixer;

        AudioConfigLoader _config;   // Configuration loader for audio settings
        bool _enabled = true;        // NEW: Sound enable flag

        int ToDxVolume(int uiVolume);
    };
}