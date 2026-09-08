#include "pch.h"

#include "AudioManager.h"

#include <algorithm>
#include <string_view>

#include "ApuVoice.h"
#include "AudioInitializer.h"
#include "BgmTransportState.h"
#include "ChannelManager.h"
#include "config/SoundConfig.h"
#include "SoundChannel.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::systems::audio
{
    AudioManager::AudioManager()
        : _bgmChannels(static_cast<int>(kApuVoiceCount)),
        _bgmManager(_bgmChannels), _seManager(),
        _mixer(_bgmManager, _seManager)
    {
        // Link BGM manager with SE manager.
        _bgmManager.SetSeManager(&_seManager);
        _seManager.SetBgmManager(&_bgmManager);
    }

    bool AudioManager::Initialize(const std::wstring& configPath)
    {
        return AudioInitializer::InitializeAudio(configPath, _bgmManager, _seManager);
    }

    bool AudioManager::Initialize(const std::wstring_view configPath)
    {
        return Initialize(std::wstring(configPath));
    }

    void AudioManager::PlayBgm(const std::wstring& name)
    {
        _bgmManager.Play(name);
    }

    void AudioManager::StopBgm()
    {
        _bgmManager.Stop();
    }

    void AudioManager::FadeOutBgm(int frames)
    {
        _bgmManager.FadeOut(frames);
    }

    void AudioManager::SetBgmVolume(int volume)
    {
        _mixer.SetBgmVolume(toDxVolume_(volume));
    }

    void AudioManager::PlaySe(const std::wstring& name)
    {
        _seManager.PlaySe(name);
    }

    void AudioManager::StopSe(const std::wstring& name)
    {
        _seManager.StopSe(name);
    }

    void AudioManager::SetSeVolume(int volume)
    {
        _mixer.SetSeVolume(toDxVolume_(volume));
    }

    void AudioManager::SetMasterVolume(int volume)
    {
        _mixer.SetMasterVolume(toDxVolume_(volume));
    }

    void AudioManager::OutputBGMMasterVolume()
    {
        for (int i = 0; i < _bgmChannels.GetChannelCount(); ++i)
        {
            int vol = _bgmChannels.GetVolume(i);
            utils::debug_log(L"[AudioManager] BGM Channel {} Master Volume: {}", i, vol);
        }
    }

    void AudioManager::MuteChannel(SoundChip chip, int index, bool mute)
    {
        auto key = std::make_pair(chip, index);
        if (mute) _mutedChannels.insert(key);
        else _mutedChannels.erase(key);
        // Add logic to actually set the channel volume to 0 or restore its original value.
    }

    void AudioManager::Pause()
    {
        _bgmManager.Pause();
        _seManager.Pause();
    }

    void AudioManager::Resume()
    {
        _bgmManager.Resume();
        _seManager.Resume();
    }

    bool AudioManager::CaptureBgmState(BgmTransportState& state) const
    {
        return _bgmManager.CaptureState(state);
    }

    bool AudioManager::ValidateBgmState(const BgmTransportState& state) const
    {
        return _bgmManager.ValidateState(state);
    }

    bool AudioManager::RestoreBgmState(const BgmTransportState& state)
    {
        return _bgmManager.RestoreState(state);
    }

    void AudioManager::SetEnabled(bool enabled)
    {
        _mixer.SetEnabled(enabled);
    }

    void AudioManager::ApplyConfig(const config::SoundConfig& cfg)
    {
        SetMasterVolume(cfg.master);
        SetBgmVolume(cfg.bgm);
        SetSeVolume(cfg.se);
        SetEnabled(cfg.enabled);
    }

    void AudioManager::Update()
    {
        _seManager.Update();
        _bgmManager.Update();
    }

    void AudioManager::Release()
    {
        _bgmManager.Release();
        _seManager.Release();
    }

    int AudioManager::toDxVolume_(int uiVolume)
    {
        uiVolume = std::clamp(uiVolume, 0, 100);
        return (uiVolume * config::SystemConfig::kAudioMaxVolume + 50) / 100;     // Convert 0-100 to 0-255 range (50 is for rounding)
    }
}
