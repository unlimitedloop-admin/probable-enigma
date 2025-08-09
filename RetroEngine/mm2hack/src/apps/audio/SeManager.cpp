#include "pch.h"

#include "ChannelManager.h"
#include "SeManager.h"

namespace mm2hack::apps::audio
{
    SeManager::SeManager(ChannelManager& bgmChannels, int seChannelCount)
        : _bgmChannels(bgmChannels), _seChannels(seChannelCount)
    {
        _bgmVolumeBackup.resize(_bgmChannels.GetChannelCount(), -1);
    }

    bool SeManager::LoadSe(const std::wstring& name, const std::vector<std::wstring>& filepath, const std::vector<int>& volume, const std::vector<int>& targetChannels)
    {
        if (name.empty() || filepath.empty()) return false;
        _seData[name] = { filepath, volume, targetChannels };
        return true;
    }

    void SeManager::PlaySe(const std::wstring& name, int overrideVolume)
    {
        auto it = _seData.find(name);
        if (it == _seData.end()) return;
        const auto& se = it->second;

        // Check if we have enough channels.
        _seChannels.EnsureChannelCount(static_cast<int>(se.filepaths.size()));

        for (size_t i = 0; i < se.filepaths.size(); ++i)
        {
            _seChannels.Load(static_cast<int>(i), se.filepaths[i]);
            int baseVol = (i < se.volumes.size()) ? se.volumes[i] : 255;
            int adjustedVol = ((overrideVolume >= 0 ? overrideVolume : baseVol) * _masterVolume) / 255;
            _seChannels.SetVolume(static_cast<int>(i), adjustedVol);
            DxLib::SetSoundCurrentPosition(0, _seChannels.GetHandle(static_cast<int>(i)));
        }

        for (size_t i = 0; i < se.filepaths.size(); ++i)
        {
            _seChannels.Play(static_cast<int>(i), false);

            // Mute BGM channel if specified.
            if (i < se.targetBgmChannels.size())
            {
                int bgmCh = se.targetBgmChannels[i];
                if (bgmCh >= 0 && bgmCh < _bgmChannels.GetChannelCount())
                {
                    if (_bgmVolumeBackup[bgmCh] == -1)
                        _bgmVolumeBackup[bgmCh] = _bgmChannels.GetVolume(bgmCh);
                    _bgmChannels.SetVolume(bgmCh, 0);
                    _activeSeChannels[bgmCh] = { name, static_cast<int>(i) };
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
        _seChannels.ResumeAll(false);   // Playing without loop
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
    }

    void SeManager::SetMasterVolume(int volume)
    {
        _masterVolume = std::clamp(volume, 0, 255);

        for (auto& [bgmCh, activeSe] : _activeSeChannels)
        {
            const auto& se = _seData[activeSe.seName];
            for (size_t i = 0; i < se.filepaths.size(); ++i)
            {
                int baseVol = (i < se.volumes.size()) ? se.volumes[i] : 255;
                int adjustedVol = (baseVol * _masterVolume) / 255;
                _seChannels.SetVolume(activeSe.seChannelIndex, adjustedVol);
            }
        }
    }
}