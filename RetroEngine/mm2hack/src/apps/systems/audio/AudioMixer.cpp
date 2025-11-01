#include "pch.h"

#include "AudioMixer.h"

namespace mm2hack::apps::systems::audio
{
    void AudioMixer::SetMasterVolume(int volume)
    {
        _masterVolume = std::clamp(volume, 0, MAX_VOLUME);
        ApplyVolumes();
    }

    void AudioMixer::SetBgmVolume(int volume)
    {
        _bgmVolume = std::clamp(volume, 0, MAX_VOLUME);
        ApplyVolumes();
    }

    void AudioMixer::SetSeVolume(int volume)
    {
        _seVolume = std::clamp(volume, 0, MAX_VOLUME);
        ApplyVolumes();
    }

    void AudioMixer::SetEnabled(bool enabled)
    {
        _enabled = enabled;
        ApplyVolumes();
    }

    void AudioMixer::FadeMaster(int target, int durationFrames)
    {
        _fading = true;
        _fadeTarget = std::clamp(target, 0, MAX_VOLUME);
        _fadeFramesRemaining = std::max(1, durationFrames);
        _fadeStep = (_fadeTarget - _masterVolume) / _fadeFramesRemaining;
    }

    void AudioMixer::Update()
    {
        // If fading is in progress, update the master volume.
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

    void AudioMixer::ApplyVolumes()
    {
        const int masterFactor = _enabled ? _masterVolume : 0;

        _bgm.SetMasterVolume((_bgmVolume * masterFactor) / MAX_VOLUME);
        _se.SetMasterVolume((_seVolume * masterFactor) / MAX_VOLUME);
    }
}