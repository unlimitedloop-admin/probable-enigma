#include "pch.h"

#include "BgmManager.h"

#include <algorithm>
#include <cstddef>

#include "ApuVoice.h"
#include "ChannelManager.h"
#include "SeManager.h"

namespace mm2hack::apps::systems::audio
{
    BgmManager::BgmManager(ChannelManager& channels)
        : _channels(channels)
    {
    }

    bool BgmManager::RegisterBgm(const std::wstring& name, const std::vector<std::wstring>& filepaths,
        const std::vector<int>& volumes, const std::vector<ApuVoice>& voices,
        double loopStart, double loopEnd)
    {
        if (name.empty() || filepaths.empty() || filepaths.size() != voices.size()) return false;
        _bgmData[name] = { filepaths, volumes, voices, loopStart, loopEnd };
        return true;
    }

    bool BgmManager::Play(const std::wstring& name)
    {
        _currentBgm = name;

        auto it = _bgmData.find(name);
        if (it == _bgmData.end()) return false;

        const auto& config = it->second;
        _channels.EnsureChannelCount(static_cast<int>(kApuVoiceCount));

        _logical_volumes.assign(kApuVoiceCount, 0);

        // Load all files and set volumes.
        for (size_t i = 0; i < config.filepaths.size(); ++i)
        {
            const int channel_index = static_cast<int>(ToIndex(config.voices[i]));
            _channels.Load(channel_index, config.filepaths[i]);

            int baseVol = (i < config.volumes.size()) ? config.volumes[i] : MAX_VOLUME;
            int adjustedVol = (baseVol * _masterVolume) / MAX_VOLUME;

            _logical_volumes[static_cast<std::size_t>(channel_index)] = adjustedVol;

            SetSoundCurrentPosition(0, _channels.GetHandle(channel_index));
        }
        RefreshOutputVolumes();

        // Play all channels with loop.
        for (size_t i = 0; i < config.filepaths.size(); ++i)
        {
            _channels.Play(static_cast<int>(ToIndex(config.voices[i])), true);
        }

        _loopStart = config.loopStart;
        _loopEnd = config.loopEnd;
        _isPlaying = true;

        SetMasterVolume(_masterVolume);

        return true;
    }

    void BgmManager::Stop()
    {
        _channels.StopAll();
        _isPlaying = false;
    }

    void BgmManager::Pause()
    {
        _channels.PauseAll();
    }

    void BgmManager::Resume()
    {
        _channels.ResumeAll(true);  // Playing with loop
    }

    void BgmManager::FadeOut(int durationFrames)
    {
        _isFading = true;
        _fadeTarget = 0;
        _fadeFramesRemaining = std::max(1, durationFrames);
        _fadeStep = (_fadeTarget - _masterVolume) / _fadeFramesRemaining;
    }

    void BgmManager::FadeIn(int durationFrames)
    {
        _isFading = true;
        _fadeTarget = MAX_VOLUME;
        _fadeFramesRemaining = std::max(1, durationFrames);
        _fadeStep = (_fadeTarget - _masterVolume) / _fadeFramesRemaining;
    }

    void BgmManager::SetMasterVolume(int volume)
    {
        _masterVolume = std::clamp(volume, 0, MAX_VOLUME);

        if (_bgmData.find(_currentBgm) == _bgmData.end()) return;
        const auto& config = _bgmData[_currentBgm];

        for (size_t i = 0; i < config.filepaths.size(); ++i)
        {
            const int channel_index = static_cast<int>(ToIndex(config.voices[i]));
            int baseVol = (i < config.volumes.size()) ? config.volumes[i] : MAX_VOLUME;
            int adjustedVol = (baseVol * _masterVolume) / MAX_VOLUME;
            _logical_volumes[static_cast<std::size_t>(channel_index)] = adjustedVol;
        }
        RefreshOutputVolumes();
    }

    void BgmManager::RefreshOutputVolumes()
    {
        const auto data_it = _bgmData.find(_currentBgm);
        if (data_it == _bgmData.end()) return;

        for (const ApuVoice voice : data_it->second.voices)
        {
            const std::size_t index = ToIndex(voice);
            const int logical_volume = index < _logical_volumes.size() ? _logical_volumes[index] : 0;
            _channels.SetVolume(
                static_cast<int>(index),
                effectiveVolume_(voice, logical_volume));
        }
    }

    void BgmManager::applyFade_()
    {
        if (_isFading && _fadeFramesRemaining > 0)
        {
            _masterVolume += _fadeStep;
            _fadeFramesRemaining--;
            SetMasterVolume(_masterVolume);

            if (_fadeFramesRemaining == 0)
            {
                SetMasterVolume(_fadeTarget);
                _isFading = false;
            }
        }
    }

    void BgmManager::checkAndApplyLoop_()
    {
        if (!_isPlaying || _loopEnd <= 0.0) return;

        const auto data_it = _bgmData.find(_currentBgm);
        if (data_it == _bgmData.end()) return;
        for (const ApuVoice voice : data_it->second.voices)
        {
            const int channel_index = static_cast<int>(ToIndex(voice));
            if (_channels.IsPlaying(channel_index))
            {
                LONGLONG posMs = DxLib::GetSoundCurrentTime(_channels.GetHandle(channel_index));
                double posSec = posMs / 1000.0;
                if (posSec >= _loopEnd)
                {
                    auto loop = static_cast<LONGLONG>(_loopStart * 1000);
                    DxLib::SetSoundCurrentTime(loop, _channels.GetHandle(channel_index));
                    //utils::debug_log(L"BGM looped: {} at channel: {}", loop, i);
                }
            }
        }
    }

    void BgmManager::Update()
    {
        applyFade_();
        checkAndApplyLoop_();
        _channels.Update();

        for (int i = 0; i < _channels.GetChannelCount(); ++i)
        {
            int vol = _channels.GetVolume(i);
        }
    }
    int BgmManager::effectiveVolume_(ApuVoice voice, int logicalVolume) const
    {
        return _seManager != nullptr && _seManager->IsVoiceOwnedBySe(voice) ? 0 : logicalVolume;
    }
}
