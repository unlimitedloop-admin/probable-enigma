#include "pch.h"

#include "AudioConfigLoader.h"
#include "ChannelManager.h"
#include "SeManager.h"
//#include "utils/output_debug.h"

namespace mm2hack::apps::systems::audio
{
    SeManager::SeManager(ChannelManager& bgmChannels, int seChannelCount)
        : _bgmChannels(bgmChannels), _seChannels(seChannelCount)
    {
        _bgmVolumeBackup.resize(_bgmChannels.GetChannelCount(), -1);
    }

    bool SeManager::LoadSe(
        const std::wstring& name,
        const std::vector<std::wstring>& filepath,
        const std::vector<int>& volume,
        const std::vector<int>& targetChannels,
        const std::vector<SePriority> priority
        )
    {
        if (name.empty() || filepath.empty()) return false;
        _seData[name] = { filepath, volume, targetChannels, priority };
        return true;
    }

    void SeManager::PlaySe(const std::wstring& name, int overrideVolume)
    {
        auto it = _seData.find(name);
        if (it == _seData.end()) return;
        const auto& se = it->second;

        if (!canPlaySe_(se))
        {
            // Cannot play due to priority.
            return;
        }
        // Check if we have enough channels.
        _seChannels.EnsureChannelCount(static_cast<int>(se.filepaths.size()));

        // NOTE: We assume that each SE consists of multiple files played simultaneously on separate channels.
        // Load and play each file, adjusting volume as needed.
        for (size_t i = 0; i < se.filepaths.size(); ++i)
        {
            int chIndex = static_cast<int>(i);
            _seChannels.Load(chIndex, se.filepaths[i]);
            int baseVol = (i < se.volumes.size()) ? se.volumes[i] : MAX_VOLUME;
            int adjustedVol = ((overrideVolume >= 0 ? overrideVolume : baseVol) * _masterVolume) / MAX_VOLUME;
            _seChannels.SetVolume(chIndex, adjustedVol);
            DxLib::SetSoundCurrentPosition(0, _seChannels.GetHandle(chIndex));
            _seChannels.Play(chIndex, false);

            _channelToSeName[chIndex] = name;

            if (i < se.targetBgmChannels.size())
            {
                int bgmCh = se.targetBgmChannels[i];
                if (bgmCh >= 0 && bgmCh < _bgmChannels.GetChannelCount())
                {
                    if (_bgmVolumeBackup[bgmCh] == -1)
                    {
                        int currentVol = _bgmManager->GetCurrentBgmVolume(bgmCh);
                        _bgmVolumeBackup[bgmCh] = currentVol;
                    }
                    _bgmChannels.SetVolume(bgmCh, 0);
                    _activeSeChannels[bgmCh] = { name, chIndex };
                }
            }
        }
    }

    void SeManager::StopAll()
    {
        _seChannels.StopAll();

        // Release all ownerships and restore BGM.
        for (auto& [bgmCh, activeSe] : _activeSeChannels)
        {
            if (_bgmVolumeBackup[bgmCh] >= 0)
            {
                _bgmChannels.SetVolume(bgmCh, _bgmVolumeBackup[bgmCh]);
                _bgmVolumeBackup[bgmCh] = -1;
            }
        }
        _activeSeChannels.clear();
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

        std::vector<int> toRelease;
        for (auto& [bgmCh, activeSe] : _activeSeChannels)
        {
            // If the SE assigned to this BGM channel has ended, restore it.
            if (!_seChannels.IsPlaying(activeSe.seChannelIndex))
            {
                if (_bgmVolumeBackup[bgmCh] >= 0)
                {
                    _bgmChannels.SetVolume(bgmCh, _bgmVolumeBackup[bgmCh]);
                    _bgmVolumeBackup[bgmCh] = -1;
                }
                toRelease.push_back(bgmCh);
            }
        }
        for (int ch : toRelease)
        {
            _activeSeChannels.erase(ch);
        }

        std::vector<int> toErase;
        for (const auto& [chIndex, name] : _channelToSeName)
        {
            if (!_seChannels.IsPlaying(chIndex))
            {
                toErase.push_back(chIndex);
            }
        }
        for (int ch : toErase)
        {
            _channelToSeName.erase(ch);
        }
    }

    void SeManager::SetMasterVolume(int volume)
    {
        _masterVolume = std::clamp(volume, 0, MAX_VOLUME);

        for (auto& [bgmCh, activeSe] : _activeSeChannels)
        {
            const auto& se = _seData[activeSe.seName];
            for (size_t i = 0; i < se.filepaths.size(); ++i)
            {
                int baseVol = (i < se.volumes.size()) ? se.volumes[i] : MAX_VOLUME;
                int adjustedVol = (baseVol * _masterVolume) / MAX_VOLUME;
                _seChannels.SetVolume(activeSe.seChannelIndex, adjustedVol);
            }
        }
    }

    SePriority SeManager::GetCurrentMaxPriority() const
    {
        SePriority maxPriority = SePriority::Low;
        for (int i = 0; i < _seChannels.GetChannelCount(); ++i)
        {
            if (_seChannels.IsPlaying(i))
            {
                auto it = _channelToSeName.find(i);
                if (it != _channelToSeName.end())
                {
                    const auto& seName = it->second;
                    auto dataIt = _seData.find(seName);
                    if (dataIt != _seData.end())
                    {
                        for (const auto& prio : dataIt->second.priority) {
                            maxPriority = std::max(maxPriority, prio);
                        }
                    }
                }
            }
        }
        return maxPriority;
    }

    bool SeManager::IsBgmChannelMuted(int index) const
    {
        if (index < 0 || index >= static_cast<int>(_bgmVolumeBackup.size()))
        {
            return false;
        }
        return _bgmVolumeBackup[index] != -1;
    }

    bool SeManager::canPlaySe_(const SeData& newSe) const
    {
        SePriority currentPriority = GetCurrentMaxPriority();
        for (const auto& prio : newSe.priority) {
            if (prio < currentPriority) {
                return false;
            }
        }
        return true;
    }
}