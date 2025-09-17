#include "pch.h"

#include "BgmManager.h"

#include "ChannelManager.h"
#include "utils/output_debug.h"

namespace mm2hack::apps::audio
{
    BgmManager::BgmManager(ChannelManager& channels)
        : _channels(channels)
    {
    }

    bool BgmManager::RegisterBgm(const std::wstring& name, const std::vector<std::wstring>& filepaths,
        const std::vector<int>& volumes, double loopStart, double loopEnd)
    {
        if (name.empty() || filepaths.empty()) return false;
        _bgmData[name] = { filepaths, volumes, loopStart, loopEnd };
        return true;
    }

    bool BgmManager::Play(const std::wstring& name)
    {
        auto it = _bgmData.find(name);
        if (it == _bgmData.end()) return false;

        const auto& config = it->second;
        for (size_t i = 0; i < config.filepaths.size(); ++i)
        {
            if (i >= static_cast<size_t>(_channels.GetChannelCount()))
                _channels.AddChannel();

            _channels.Load(static_cast<int>(i), config.filepaths[i]);
            int baseVol = (i < config.volumes.size()) ? config.volumes[i] : MAX_VOLUME;
            int adjustedVol = (baseVol * _masterVolume) / MAX_VOLUME;
            _channels.SetVolume(static_cast<int>(i), adjustedVol);
            SetSoundCurrentPosition(0, _channels.GetHandle(static_cast<int>(i)));
        }

        // NOTE: To play the audio data synchronously, only the Play call is expanded in a loop.
        for (size_t i = 0; i < config.filepaths.size(); ++i)
        {
            _channels.Play(static_cast<int>(i), true);
        }

        _loopStart = config.loopStart;
        _loopEnd = config.loopEnd;
        _isPlaying = true;
        _currentBgm = name;
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
            int baseVol = (i < config.volumes.size()) ? config.volumes[i] : MAX_VOLUME;
            int adjustedVol = (baseVol * _masterVolume) / MAX_VOLUME;
            _channels.SetVolume(static_cast<int>(i), adjustedVol);
            utils::debug_log(L"change BGM vol: {}, at channel: {}", adjustedVol, i);
        }
    }

    void BgmManager::ApplyFade()
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

    void BgmManager::CheckAndApplyLoop()
    {
        if (!_isPlaying || _loopEnd <= 0.0) return;

        for (int i = 0; i < _channels.GetChannelCount(); ++i)
        {
            if (_channels.IsPlaying(i))
            {
                LONGLONG posMs = DxLib::GetSoundCurrentTime(_channels.GetHandle(i));
                double posSec = posMs / 1000.0;
                if (posSec >= _loopEnd)
                {
                    auto loop = static_cast<LONGLONG>(_loopStart * 1000);
                    DxLib::SetSoundCurrentTime(loop, _channels.GetHandle(i));
                    utils::debug_log(L"BGM looped: {} at channel: {}", loop, i);
                }
            }
        }
    }

    void BgmManager::Update()
    {
        ApplyFade();
        CheckAndApplyLoop();
        _channels.Update();

        for (int i = 0; i < _channels.GetChannelCount(); ++i)
        {
            int vol = _channels.GetVolume(i);
        }
    }
}