#include "pch.h"

#include "SoundChannel.h"

namespace mm2hack::apps::audio
{
    SoundChannel::SoundChannel() = default;

    SoundChannel::~SoundChannel()
    {
        if (_handle != -1)
        {
            DeleteSoundMem(_handle);
        }
    }

    bool SoundChannel::Load(const std::wstring& filepath)
    {
        if (_handle != -1)
        {
            DeleteSoundMem(_handle);
        }
        _handle = LoadSoundMem(filepath.c_str());
        return _handle != -1;
    }

    void SoundChannel::Play(bool loop)
    {
        if (_handle != -1)
        {
            PlaySoundMem(_handle, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
        }
    }

    void SoundChannel::Stop()
    {
        if (_handle != -1)
        {
            StopSoundMem(_handle);
        }
    }

    void SoundChannel::SetVolume(int volume)
    {
        _volume = std::clamp(volume, 0, 255);
        if (_handle != -1)
        {
            ChangeVolumeSoundMem(_volume, _handle);
        }
    }

    int SoundChannel::GetVolume() const
    {
        return _volume;
    }

    bool SoundChannel::IsPlaying() const
    {
        return (_handle != -1 && CheckSoundMem(_handle) == 1);
    }

    void SoundChannel::StartFade(int targetVolume, int durationFrames)
    {
        _fade_target = std::clamp(targetVolume, 0, 255);
        _fade_frames_remaining = std::max(1, durationFrames);
        int diff = _fade_target - _volume;
        _fade_step = diff / _fade_frames_remaining;
    }

    void SoundChannel::Update()
    {
        if (_fade_frames_remaining > 0)
        {
            _volume += _fade_step;
            _fade_frames_remaining--;
            SetVolume(_volume);
            if (_fade_frames_remaining == 0)
            {
                SetVolume(_fade_target);
            }
        }
    }
}