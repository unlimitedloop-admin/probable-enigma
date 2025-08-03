#include "pch.h"

#include "AudioInitializer.h"
#include "AudioManager.h"
#include "ChannelManager.h"

namespace mm2hack::apps::audio
{
    AudioManager::AudioManager()
        : _bgmChannels(5), _seChannels(8),
        _bgmManager(_bgmChannels), _seManager(_bgmChannels, 8),
        _mixer(_bgmManager, _seManager)
    {
    }

    bool AudioManager::Initialize(const std::wstring& configPath)
    {
        return AudioInitializer::InitializeAudio(configPath, _bgmManager, _seManager, _bgmChannels);
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
        _mixer.SetBgmVolume(ToDxVolume(volume));
    }

    void AudioManager::PlaySe(const std::wstring& name)
    {
        _seManager.PlaySe(name);
    }

    void AudioManager::SetSeVolume(int volume)
    {
        _mixer.SetSeVolume(ToDxVolume(volume));
    }

    void AudioManager::SetMasterVolume(int volume)
    {
        _mixer.SetMasterVolume(ToDxVolume(volume));
    }

    void AudioManager::SetEnabled(bool enabled)
    {
        _mixer.SetEnabled(enabled);
    }

    void AudioManager::Update()
    {
        _bgmManager.Update();
        _seManager.Update();
    }

    void AudioManager::Release()
    {
        _bgmManager.Stop();
        _seManager.StopAll();

        _bgmChannels.Clear();
        _seChannels.Clear();
    }

    int AudioManager::ToDxVolume(int uiVolume)
    {
        uiVolume = std::clamp(uiVolume, 0, 100);
        return (uiVolume * 255 + 50) / 100;     // Convert 0-100 to 0-255 range (50 is for rounding)
    }
}