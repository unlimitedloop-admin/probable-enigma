#include "pch.h"

#include "AudioMixer.h"

namespace mm2hack::apps::audio
{
    void AudioMixer::SetMasterVolume(int volume)
    {
        _masterVolume = std::clamp(volume, 0, 255);
        _bgm.SetMasterVolume((_bgmVolume * _masterVolume) / 255);
        _se.SetMasterVolume((_seVolume * _masterVolume) / 255);
    }

    void AudioMixer::SetBgmVolume(int volume)
    {
        _bgmVolume = std::clamp(volume, 0, 255);
        _bgm.SetMasterVolume((_bgmVolume * _masterVolume) / 255);
    }

    void AudioMixer::SetSeVolume(int volume)
    {
        _seVolume = std::clamp(volume, 0, 255);
        _se.SetMasterVolume((_seVolume * _masterVolume) / 255);
    }

    void AudioMixer::FadeMaster(int target, int durationFrames)
    {
        _fading = true;
        _fadeTarget = std::clamp(target, 0, 255);
        _fadeFramesRemaining = std::max(1, durationFrames);
        _fadeStep = (_fadeTarget - _masterVolume) / _fadeFramesRemaining;
    }

    void AudioMixer::Update()
    {
        if (_fading && _fadeFramesRemaining > 0)
        {
            _masterVolume += _fadeStep;
            _fadeFramesRemaining--;
            SetMasterVolume(_masterVolume);
            if (_fadeFramesRemaining == 0)
            {
                SetMasterVolume(_fadeTarget);
                _fading = false;
            }
        }
    }
}