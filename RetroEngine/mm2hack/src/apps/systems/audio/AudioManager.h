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
#include <string_view>
#include <utility>
#include "AudioConfigLoader.h"
#include "AudioMixer.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "config/SoundConfig.h"
#include "SeManager.h"
#include "SoundChannel.h"

namespace mm2hack::apps::systems::audio
{
    // AudioManager is the main class that manages BGM and SE playback
    class AudioManager
    {
    public:
        AudioManager();
        ~AudioManager() = default;

        // Initialize audio system with configuration file
        bool Initialize(const std::wstring& configPath);
        // Initialize audio system with configuration file (wstring_view overload)
        bool Initialize(const std::wstring_view configPath);

        // ==== Controlling BGM ====
        // Play the background music by name
        void PlayBgm(const std::wstring& name);
        // Stop the background music
        void StopBgm();
        // Fade out the background music over a specified number of frames
        void FadeOutBgm(int frames);
        // Set the BGM volume (0-100)
        void SetBgmVolume(int volume);

        // ==== Controlling SE ====
        // Play the sound effect by name
        void PlaySe(const std::wstring& name);
        // Set the SE volume (0-100)
        void SetSeVolume(int volume);

        // Master volume leveling
        void SetMasterVolume(int volume);
        // Mute for channels
        void MuteChannel(SoundChip chip, int index, bool mute);

        // Output current BGM master volume to debug log
        void OutputBGMMasterVolume();

        // Pause a all sounds
        void Pause();
        // Resume a all sounds
        void Resume();

        // Enable / disable all sounds
        void SetEnabled(bool enabled);
        bool IsEnabled() const { return _enabled; }

        // Load configuration from file
        void ApplyConfig(const config::SoundConfig& cfg);

        // Every updates
        void Update();
        // Release all resources, stop all sounds
        void Release();

        // Callbacks for sound events (e.g., when BGM starts/stops)
        std::function<void(const std::wstring&)> OnBgmStarted;
        std::function<void(const std::wstring&)> OnBgmStopped;
        std::function<void(const std::wstring&)> OnSeStarted;
        std::function<void(const std::wstring&)> OnSeStopped;

    private:
        int toDxVolume_(int uiVolume);  // Convert volume from 0-100 range to 0-255 range used by the audio system

    private:
        const std::wstring kClassName{ L"AudioManager" };

        // ==== Sound engine components ====
        ChannelManager _bgmChannels;                        // BGM channel manager
        ChannelManager _seChannels;                         // SE channel manager
        BgmManager _bgmManager;                             // BGM manager
        SeManager _seManager;                               // SE manager
        AudioMixer _mixer;                                  // Audio mixer
        AudioConfigLoader _config;                          // Audio configuration loader
        std::set<std::pair<SoundChip, int>> _mutedChannels; // Set of muted channels (chip, index)

        bool _enabled = true;
    };
}