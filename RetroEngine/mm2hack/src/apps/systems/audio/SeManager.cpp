#include "pch.h"

#include "SeManager.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "ApuVoice.h"
#include "ApuVoiceArbiter.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "SePriority.h"

namespace mm2hack::apps::systems::audio
{
    SeManager::SeManager()
        : _seChannels(static_cast<int>(kApuVoiceCount))
    {
    }

    bool SeManager::LoadSe(
        const std::wstring& name,
        const std::vector<std::wstring>& filepath,
        const std::vector<int>& volume,
        const std::vector<ApuVoice>& voices,
        const std::vector<SePriority> priority,
        double loopStart,
        double loopEnd
        )
    {
        if (name.empty() || filepath.empty() || filepath.size() != voices.size() ||
            (!priority.empty() && filepath.size() != priority.size()))
        {
            return false;
        }

        std::array<bool, kApuVoiceCount> occupied{};
        for (const ApuVoice voice : voices)
        {
            const std::size_t index = ToIndex(voice);
            if (index >= occupied.size() || occupied[index])
            {
                return false;
            }
            occupied[index] = true;
        }

        std::vector<SePriority> priorities = priority;
        if (priorities.empty())
        {
            priorities.assign(filepath.size(), SePriority::Normal);
        }
        _seData[name] = { filepath, volume, voices, std::move(priorities), loopStart, loopEnd };
        return true;
    }

    void SeManager::StopSe(const std::wstring& name)
    {
        stopPlaybackForOwners_({ name });
        _voiceArbiter.Release(name);
        applyBgmOwnership_();
    }

    void SeManager::PlaySe(const std::wstring& name, int overrideVolume)
    {
        auto it = _seData.find(name);
        if (it == _seData.end()) return;
        const auto& se = it->second;

        std::vector<ApuVoiceClaim> claims;
        claims.reserve(se.voices.size());
        for (std::size_t index = 0; index < se.voices.size(); ++index)
        {
            claims.push_back({ se.voices[index], se.priority[index] });
        }

        const ApuVoiceAcquisition acquisition = _voiceArbiter.Acquire(name, claims);
        if (!acquisition.accepted) return;

        stopPlaybackForOwners_(acquisition.displacedOwners);
        _seChannels.EnsureChannelCount(static_cast<int>(kApuVoiceCount));

        // NOTE: We assume that each SE consists of multiple files played simultaneously on separate channels.
        // Load and play each file, adjusting volume as needed.
        for (std::size_t i = 0; i < se.filepaths.size(); ++i)
        {
            const int chIndex = static_cast<int>(ToIndex(se.voices[i]));
            if (!_seChannels.Load(chIndex, se.filepaths[i]))
            {
                StopSe(name);
                return;
            }
            int baseVol = (i < se.volumes.size()) ? se.volumes[i] : MAX_VOLUME;
            int adjustedVol = ((overrideVolume >= 0 ? overrideVolume : baseVol) * _masterVolume) / MAX_VOLUME;
            _seChannels.SetVolume(chIndex, adjustedVol);
            _seChannels.SetPositionMilliseconds(chIndex, 0);
            _seChannels.Play(chIndex, false);

            _channelToSeName[chIndex] = name;
        }
        applyBgmOwnership_();
    }

    void SeManager::StopAll()
    {
        _seChannels.StopAll();
        _channelToSeName.clear();
        _voiceArbiter.Clear();
        applyBgmOwnership_();
    }

    void SeManager::Release()
    {
        StopAll();
        _seChannels.Clear();
        _seData.clear();
    }

    void SeManager::Pause()
    {
        _seChannels.PauseAll();
    }

    void SeManager::Resume()
    {
        // Playing without loop.
        _seChannels.ResumeAll(false);
    }

    void SeManager::Update()
    {
        _seChannels.Update();

        for (const auto& [channel_index, name] : _channelToSeName)
        {
            const auto data_it = _seData.find(name);
            if (data_it == _seData.end()) continue;

            const auto& se = data_it->second;
            if (se.loopEnd <= se.loopStart || se.loopEnd <= 0.0 ||
                !_seChannels.IsPlaying(channel_index))
            {
                continue;
            }

            const auto position_ms = _seChannels.GetPositionMilliseconds(channel_index);
            const auto loop_end_ms = static_cast<std::int64_t>(se.loopEnd * 1000.0);
            if (position_ms >= loop_end_ms)
            {
                const auto loop_start_ms = static_cast<std::int64_t>(se.loopStart * 1000.0);
                _seChannels.SetPositionMilliseconds(channel_index, loop_start_ms);
            }
        }

        bool ownership_changed = false;
        for (std::size_t index = 0; index < kApuVoiceCount; ++index)
        {
            const ApuVoice voice = static_cast<ApuVoice>(index);
            const ApuVoiceOwner* owner = _voiceArbiter.GetOwner(voice);
            if (owner != nullptr && !_seChannels.IsPlaying(static_cast<int>(index)))
            {
                const std::wstring owner_name = owner->name;
                _voiceArbiter.Release(voice, owner_name);
                _channelToSeName.erase(static_cast<int>(index));
                ownership_changed = true;
            }
        }
        if (ownership_changed)
        {
            applyBgmOwnership_();
        }
    }

    void SeManager::SetMasterVolume(int volume)
    {
        _masterVolume = std::clamp(volume, 0, MAX_VOLUME);

        for (std::size_t index = 0; index < kApuVoiceCount; ++index)
        {
            const ApuVoice voice = static_cast<ApuVoice>(index);
            const ApuVoiceOwner* owner = _voiceArbiter.GetOwner(voice);
            if (owner == nullptr) continue;

            const auto data_it = _seData.find(owner->name);
            if (data_it == _seData.end()) continue;
            const SeData& se = data_it->second;
            for (std::size_t stem = 0; stem < se.voices.size(); ++stem)
            {
                if (se.voices[stem] != voice) continue;
                int baseVol = (stem < se.volumes.size()) ? se.volumes[stem] : MAX_VOLUME;
                int adjustedVol = (baseVol * _masterVolume) / MAX_VOLUME;
                _seChannels.SetVolume(static_cast<int>(index), adjustedVol);
                break;
            }
        }
    }

    SePriority SeManager::GetCurrentMaxPriority() const
    {
        return _voiceArbiter.GetCurrentMaxPriority();
    }

    bool SeManager::IsVoiceOwnedBySe(ApuVoice voice) const
    {
        return _voiceArbiter.IsOwned(voice);
    }

    void SeManager::applyBgmOwnership_()
    {
        if (_bgmManager == nullptr) return;
        _bgmManager->RefreshOutputVolumes();
    }

    void SeManager::stopPlaybackForOwners_(const std::vector<std::wstring>& owners)
    {
        std::vector<int> channels_to_release;
        for (const auto& [channel_index, owner] : _channelToSeName)
        {
            if (std::find(owners.begin(), owners.end(), owner) != owners.end())
            {
                channels_to_release.push_back(channel_index);
            }
        }

        for (const int channel_index : channels_to_release)
        {
            _seChannels.Stop(channel_index);
            _channelToSeName.erase(channel_index);
        }
    }
}
