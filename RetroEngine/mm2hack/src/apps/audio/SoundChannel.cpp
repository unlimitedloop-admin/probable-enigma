#include "pch.h"

#include "SoundChannel.h"

namespace mm2hack::apps::audio
{
    SoundChannel::SoundChannel() = default;

    SoundChannel::~SoundChannel()
    {
        if (_handle != -1)
        {
            DxLib::DeleteSoundMem(_handle);
            _handle = -1;
        }
    }

    bool SoundChannel::Load(const std::wstring& filepath)
    {
        if (_handle != -1)
        {
            DxLib::DeleteSoundMem(_handle);
            _handle = -1;
        }
        _handle = DxLib::LoadSoundMem(filepath.c_str());
        return _handle != -1;
    }

    void SoundChannel::Play(bool loop)
    {
        if (_handle != -1)
        {
            DxLib::PlaySoundMem(_handle, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK);
        }
    }

    void SoundChannel::Stop()
    {
        if (_handle != -1)
        {
            DxLib::StopSoundMem(_handle);
        }
    }

    void SoundChannel::Pause()
    {
        if (_handle != -1)
        {
            if (DxLib::CheckSoundMem(_handle) == 1)
            {
                _pausedPos = DxLib::GetSoundCurrentPosition(_handle);
                DxLib::StopSoundMem(_handle);
                _wasPaused = true;
            }
            else
            {
                _wasPaused = false;
            }
        }
    }

    void SoundChannel::Resume(bool loop)
    {
        if (_handle != -1 && _wasPaused)
        {
            DxLib::SetSoundCurrentPosition(_pausedPos, _handle);
            DxLib::PlaySoundMem(_handle, loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK, FALSE);
            _wasPaused = false;
        }
    }

    void SoundChannel::SetVolume(int volume)
    {
        _volume = std::clamp(volume, 0, 255);
        if (_handle != -1)
        {
            auto i = DxLib::ChangeVolumeSoundMem(_volume, _handle);
        }
    }

    int SoundChannel::GetVolume() const
    {
        return _volume;
    }

    bool SoundChannel::IsPlaying() const
    {
        return (_handle != -1 && DxLib::CheckSoundMem(_handle) == 1);
    }

    LONGLONG SoundChannel::GetPosition() const
    {
        return (_handle != -1) ? DxLib::GetSoundCurrentPosition(_handle) : 0LL;
    }

    void SoundChannel::SetPosition(LONGLONG pos) const
    {
        if (_handle != -1)
        {
            DxLib::SetSoundCurrentPosition(pos, _handle);
        }
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