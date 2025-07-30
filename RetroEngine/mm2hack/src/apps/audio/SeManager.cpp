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

    bool SeManager::LoadSe(const std::wstring& name, const std::wstring& filepath, int volume, const std::vector<int>& targetChannels)
    {
        if (name.empty() || filepath.empty()) return false;
        _seData[name] = { filepath, volume, targetChannels };
        return true;
    }

    void SeManager::PlaySe(const std::wstring& name, int volume)
    {
        auto it = _seData.find(name);
        if (it == _seData.end()) return;
        const auto& se = it->second;

        for (int ch : se.targetBgmChannels)
        {
            if (_activeSeChannels.count(ch))
            {
                int seIdx = _activeSeChannels[ch].seChannelIndex;
                if (_seChannels.IsPlaying(seIdx))
                {
                    _seChannels.Stop(seIdx);
                }
                _activeSeChannels.erase(ch);
            }
        }

        // 1. Find a free SE channel (if none, overwrite the oldest one)
        int seChannelIndex = -1;
        for (int i = 0; i < _seChannels.GetChannelCount(); ++i)
        {
            if (!_seChannels.IsPlaying(i))
            {
                seChannelIndex = i;
                break;
            }
        }
        if (seChannelIndex == -1)
        {
            seChannelIndex = 0;
            _seChannels.Stop(seChannelIndex);
        }

        // 2. Load and play SE
        _seChannels.Load(seChannelIndex, se.filepath);
        int vol = (volume >= 0) ? volume : se.volume;
        _seChannels.SetVolume(seChannelIndex, vol);
        _seChannels.Play(seChannelIndex, false);

        // 3. Immediately mute target BGM channels and update ownership
        for (int ch : se.targetBgmChannels)
        {
            if (ch >= 0 && ch < _bgmChannels.GetChannelCount())
            {
                // Save backup only the first time
                if (_bgmVolumeBackup[ch] == -1)
                {
                    _bgmVolumeBackup[ch] = _bgmChannels.GetVolume(ch);
                }
                // Immediately mute BGM channel
                _bgmChannels.SetVolume(ch, 0);

                // Update ownership mapping (last one wins)
                _activeSeChannels[ch] = { name, seChannelIndex };
            }
        }
    }

    void SeManager::StopAll()
    {
        _seChannels.StopAll();

        // Release all ownerships and restore BGM
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

    void SeManager::Update()
    {
        _seChannels.Update();

        std::vector<int> toRelease;
        for (auto& [bgmCh, activeSe] : _activeSeChannels)
        {
            // If the SE assigned to this BGM channel has ended, restore it
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
        for (int i = 0; i < _seChannels.GetChannelCount(); ++i)
        {
            if (_seChannels.IsPlaying(i))
                _seChannels.SetVolume(i, _masterVolume);
        }
    }
}