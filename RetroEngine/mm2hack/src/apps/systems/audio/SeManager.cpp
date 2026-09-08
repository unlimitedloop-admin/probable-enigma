#include "pch.h"

#include "SeManager.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ApuVoice.h"
#include "ApuVoiceArbiter.h"
#include "BgmManager.h"
#include "ChannelManager.h"
#include "ISoundChannel.h"
#include "SePriority.h"
#include "SeRestorePolicy.h"
#include "SeTransportState.h"
#include "SoundChannel.h"

namespace mm2hack::apps::systems::audio
{
    SeManager::SeManager()
        : SeManager([]() { return std::make_unique<SoundChannel>(); })
    {
    }

    SeManager::SeManager(SoundChannelFactory channelFactory)
        : _seChannels(0)
    {
        if (!channelFactory) return;
        for (std::size_t index = 0; index < kApuVoiceCount; ++index)
        {
            std::unique_ptr<ISoundChannel> channel = channelFactory();
            if (!channel)
            {
                channel = std::make_unique<SoundChannel>();
            }
            _seChannels.AddChannel(std::move(channel));
        }
    }

    bool SeManager::LoadSe(
        const std::wstring& name,
        const std::vector<std::wstring>& filepath,
        const std::vector<int>& volume,
        const std::vector<ApuVoice>& voices,
        const std::vector<SePriority> priority,
        double loopStart,
        double loopEnd,
        SeRestorePolicy restorePolicy
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
        if (restorePolicy == SeRestorePolicy::Continuous &&
            (loopEnd <= loopStart || loopEnd <= 0.0))
        {
            return false;
        }
        _seData[name] = {
            filepath,
            volume,
            voices,
            std::move(priorities),
            loopStart,
            loopEnd,
            restorePolicy
        };
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
            _logicalVolumes[static_cast<std::size_t>(chIndex)] =
                overrideVolume >= 0 ? std::clamp(overrideVolume, 0, MAX_VOLUME) : baseVol;
            _pausedVoices[static_cast<std::size_t>(chIndex)] = false;
            applyVoiceVolume_(se.voices[i]);
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
        _logicalVolumes.fill(0);
        _pausedVoices.fill(false);
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
        for (const auto& [channel_index, name] : _channelToSeName)
        {
            (void)name;
            if (_seChannels.IsPlaying(channel_index))
            {
                _seChannels.Pause(channel_index);
                _pausedVoices[static_cast<std::size_t>(channel_index)] = true;
            }
        }
    }

    void SeManager::Resume()
    {
        for (const auto& [channel_index, name] : _channelToSeName)
        {
            (void)name;
            const std::size_t index = static_cast<std::size_t>(channel_index);
            if (_pausedVoices[index])
            {
                _seChannels.Resume(channel_index, false);
                _pausedVoices[index] = false;
            }
        }
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
                _pausedVoices[static_cast<std::size_t>(channel_index)] ||
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
            if (owner != nullptr && !_pausedVoices[index] &&
                !_seChannels.IsPlaying(static_cast<int>(index)))
            {
                const std::wstring owner_name = owner->name;
                _voiceArbiter.Release(voice, owner_name);
                _channelToSeName.erase(static_cast<int>(index));
                _logicalVolumes[index] = 0;
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

            applyVoiceVolume_(voice);
        }
    }

    bool SeManager::CaptureState(SeTransportState& state) const
    {
        SeTransportState result{};
        result.master_volume = _masterVolume;
        std::unordered_set<std::wstring> captured_names;

        for (std::size_t voice_index = 0; voice_index < kApuVoiceCount; ++voice_index)
        {
            const ApuVoice voice = static_cast<ApuVoice>(voice_index);
            const ApuVoiceOwner* owner = _voiceArbiter.GetOwner(voice);
            if (owner == nullptr || captured_names.contains(owner->name)) continue;

            const auto data_it = _seData.find(owner->name);
            if (data_it == _seData.end()) return false;
            const SeData& data = data_it->second;
            captured_names.emplace(owner->name);
            if (data.restorePolicy != SeRestorePolicy::Continuous) continue;

            ContinuousSeTransportState instance{};
            instance.name = owner->name;
            bool paused = false;
            bool pause_status_set = false;
            for (const ApuVoice configured_voice : data.voices)
            {
                const std::size_t index = ToIndex(configured_voice);
                const ApuVoiceOwner* configured_owner = _voiceArbiter.GetOwner(configured_voice);
                const auto channel_it = _channelToSeName.find(static_cast<int>(index));
                if (configured_owner == nullptr || configured_owner->name != owner->name ||
                    channel_it == _channelToSeName.end() || channel_it->second != owner->name)
                {
                    return false;
                }
                if (pause_status_set && paused != _pausedVoices[index]) return false;
                paused = _pausedVoices[index];
                pause_status_set = true;
                instance.voices.push_back({
                    configured_voice,
                    _seChannels.GetPositionMilliseconds(static_cast<int>(index)),
                    _logicalVolumes[index]
                    });
            }
            instance.playback_status = paused ?
                SePlaybackStatus::Paused : SePlaybackStatus::Playing;
            result.continuous_instances.push_back(std::move(instance));
        }

        if (!ValidateState(result)) return false;
        state = std::move(result);
        return true;
    }

    bool SeManager::ValidateState(const SeTransportState& state) const
    {
        if (!state.IsValid()) return false;
        for (const auto& instance : state.continuous_instances)
        {
            const auto data_it = _seData.find(instance.name);
            if (data_it == _seData.end() ||
                data_it->second.restorePolicy != SeRestorePolicy::Continuous ||
                data_it->second.voices.size() != instance.voices.size())
            {
                return false;
            }
            for (const ApuVoice configured_voice : data_it->second.voices)
            {
                const auto saved_voice = std::find_if(
                    instance.voices.begin(), instance.voices.end(),
                    [configured_voice](const SeVoiceTransportState& voice_state)
                    {
                        return voice_state.voice == configured_voice;
                    });
                if (saved_voice == instance.voices.end()) return false;
            }
        }
        return true;
    }

    bool SeManager::RestoreState(const SeTransportState& state)
    {
        if (!ValidateState(state)) return false;

        StopAll();
        _masterVolume = state.master_volume;
        for (const auto& instance : state.continuous_instances)
        {
            PlaySe(instance.name);
            for (const auto& voice_state : instance.voices)
            {
                const std::size_t index = ToIndex(voice_state.voice);
                const ApuVoiceOwner* owner = _voiceArbiter.GetOwner(voice_state.voice);
                if (owner == nullptr || owner->name != instance.name)
                {
                    StopAll();
                    return false;
                }
                _logicalVolumes[index] = voice_state.logical_volume;
                _seChannels.SetPositionMilliseconds(
                    static_cast<int>(index), voice_state.position_milliseconds);
                applyVoiceVolume_(voice_state.voice);
                if (instance.playback_status == SePlaybackStatus::Paused)
                {
                    _seChannels.Pause(static_cast<int>(index));
                    _pausedVoices[index] = true;
                }
            }
        }
        applyBgmOwnership_();
        return true;
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

    void SeManager::applyVoiceVolume_(ApuVoice voice)
    {
        const std::size_t index = ToIndex(voice);
        if (index >= _logicalVolumes.size()) return;
        const int adjusted_volume = (_logicalVolumes[index] * _masterVolume) / MAX_VOLUME;
        _seChannels.SetVolume(static_cast<int>(index), adjusted_volume);
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
            _logicalVolumes[static_cast<std::size_t>(channel_index)] = 0;
            _pausedVoices[static_cast<std::size_t>(channel_index)] = false;
            _channelToSeName.erase(channel_index);
        }
    }
}
