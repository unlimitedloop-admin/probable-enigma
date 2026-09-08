#include "pch.h"

#include "BgmManager.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "ApuVoice.h"
#include "BgmTransportState.h"
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
        auto it = _bgmData.find(name);
        if (it == _bgmData.end()) return false;

        const auto& config = it->second;
        _channels.EnsureChannelCount(static_cast<int>(kApuVoiceCount));
        _channels.StopAll();

        _logical_volumes.assign(kApuVoiceCount, 0);

        // Load all files and set volumes.
        for (size_t i = 0; i < config.filepaths.size(); ++i)
        {
            const int channel_index = static_cast<int>(ToIndex(config.voices[i]));
            if (!_channels.Load(channel_index, config.filepaths[i]))
            {
                _channels.StopAll();
                _isPlaying = false;
                _isPaused = false;
                return false;
            }

            int baseVol = (i < config.volumes.size()) ? config.volumes[i] : MAX_VOLUME;
            int adjustedVol = (baseVol * _masterVolume) / MAX_VOLUME;

            _logical_volumes[static_cast<std::size_t>(channel_index)] = adjustedVol;

            _channels.SetPositionMilliseconds(channel_index, 0);
        }
        _currentBgm = name;
        RefreshOutputVolumes();

        // Play all channels with loop.
        for (size_t i = 0; i < config.filepaths.size(); ++i)
        {
            _channels.Play(static_cast<int>(ToIndex(config.voices[i])), true);
        }

        _loopStart = config.loopStart;
        _loopEnd = config.loopEnd;
        _isPlaying = true;
        _isPaused = false;

        SetMasterVolume(_masterVolume);

        return true;
    }

    void BgmManager::Stop()
    {
        _channels.StopAll();
        _isPlaying = false;
        _isPaused = false;
        _isFading = false;
        _fadeFramesRemaining = 0;
    }

    void BgmManager::Release()
    {
        Stop();
        _channels.Clear();
        _bgmData.clear();
        _currentBgm.clear();
        _logical_volumes.clear();
        _loopStart = 0.0;
        _loopEnd = 0.0;
        _isFading = false;
        _fadeFramesRemaining = 0;
    }

    void BgmManager::Pause()
    {
        if (!_isPlaying || _isPaused) return;
        _channels.PauseAll();
        _isPaused = true;
    }

    void BgmManager::Resume()
    {
        if (!_isPlaying || !_isPaused) return;
        _channels.ResumeAll(true);  // Playing with loop
        _isPaused = false;
    }

    bool BgmManager::CaptureState(BgmTransportState& state) const
    {
        BgmTransportState result{};
        result.master_volume = _masterVolume;
        result.fade_target = _fadeTarget;
        if (!_isPlaying)
        {
            state = std::move(result);
            return state.IsValid();
        }

        const auto data_it = _bgmData.find(_currentBgm);
        if (data_it == _bgmData.end()) return false;

        result.track_name = _currentBgm;
        result.playback_status = _isPaused ?
            BgmPlaybackStatus::Paused : BgmPlaybackStatus::Playing;
        result.loop_start_seconds = _loopStart;
        result.loop_end_seconds = _loopEnd;
        result.is_fading = _isFading;
        result.fade_target = _fadeTarget;
        result.fade_step = _fadeStep;
        result.fade_frames_remaining = _fadeFramesRemaining;
        result.voices.reserve(data_it->second.voices.size());
        for (const ApuVoice voice : data_it->second.voices)
        {
            const std::size_t index = ToIndex(voice);
            if (index >= _logical_volumes.size()) return false;
            result.voices.push_back({
                voice,
                _channels.GetPositionMilliseconds(static_cast<int>(index)),
                _logical_volumes[index]
                });
        }
        if (!ValidateState(result)) return false;
        state = std::move(result);
        return true;
    }

    bool BgmManager::ValidateState(const BgmTransportState& state) const
    {
        if (!state.IsValid()) return false;
        if (state.playback_status == BgmPlaybackStatus::Stopped) return true;

        const auto data_it = _bgmData.find(state.track_name);
        if (data_it == _bgmData.end() ||
            data_it->second.voices.size() != state.voices.size() ||
            data_it->second.loopStart != state.loop_start_seconds ||
            data_it->second.loopEnd != state.loop_end_seconds)
        {
            return false;
        }

        for (const ApuVoice configured_voice : data_it->second.voices)
        {
            const auto saved_voice = std::find_if(
                state.voices.begin(), state.voices.end(),
                [configured_voice](const BgmVoiceTransportState& voice_state)
                {
                    return voice_state.voice == configured_voice;
                });
            if (saved_voice == state.voices.end()) return false;
        }
        return true;
    }

    bool BgmManager::RestoreState(const BgmTransportState& state)
    {
        if (!ValidateState(state)) return false;
        if (state.playback_status == BgmPlaybackStatus::Stopped)
        {
            Stop();
            _masterVolume = state.master_volume;
            _fadeTarget = state.fade_target;
            return true;
        }
        if (!Play(state.track_name)) return false;

        _masterVolume = state.master_volume;
        _loopStart = state.loop_start_seconds;
        _loopEnd = state.loop_end_seconds;
        _isFading = state.is_fading;
        _fadeTarget = state.fade_target;
        _fadeStep = state.fade_step;
        _fadeFramesRemaining = state.fade_frames_remaining;
        for (const auto& voice_state : state.voices)
        {
            const std::size_t index = ToIndex(voice_state.voice);
            _logical_volumes[index] = voice_state.logical_volume;
            _channels.SetPositionMilliseconds(
                static_cast<int>(index), voice_state.position_milliseconds);
        }
        RefreshOutputVolumes();
        if (state.playback_status == BgmPlaybackStatus::Paused)
        {
            Pause();
        }
        return true;
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
                const std::int64_t position_ms =
                    _channels.GetPositionMilliseconds(channel_index);
                double posSec = position_ms / 1000.0;
                if (posSec >= _loopEnd)
                {
                    const auto loop = static_cast<std::int64_t>(_loopStart * 1000);
                    _channels.SetPositionMilliseconds(channel_index, loop);
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
