//==============================================================================
// 
//  Project: mm2hack
//  AudioManager.h
// 
//  A class that manages the execution control of all sound sources.
// 
//==============================================================================
#pragma once

#include <functional>
#include <set>
#include <string>
#include <utility>
#include "AudioConfigLoader.h"
#include "AudioMixer.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "config/SoundConfig.h"
#include "SeManager.h"
#include "SoundChannel.h"

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

        // Mute for channels
        void MuteChannel(SoundChip chip, int index, bool mute);

        // Pause / resume BGM and SE
        void Pause();
        void Resume();

        // Enable / disable all sounds
        void SetEnabled(bool enabled);
        bool IsEnabled() const { return _enabled; }

        // Load configuration from file
        void ApplyConfig(const config::SoundConfig& cfg);

        // Every updates
        void Update();

        void Release();

        // Callbacks for sound events (e.g., when BGM starts/stops)
        std::function<void(const std::wstring&)> OnBgmStarted;
        std::function<void(const std::wstring&)> OnBgmStopped;
        std::function<void(const std::wstring&)> OnSeStarted;
        std::function<void(const std::wstring&)> OnSeStopped;

    private:
        ChannelManager _bgmChannels;
        ChannelManager _seChannels;
        BgmManager _bgmManager;
        SeManager _seManager;
        AudioMixer _mixer;

        AudioConfigLoader _config;
        bool _enabled = true;

        std::set<std::pair<SoundChip, int>> _mutedChannels; // Set of muted channels (chip, index)

        // Convert volume from 0-100 range to 0-255 range used by the audio system
        int ToDxVolume(int uiVolume);
    };
}