#include "pch.h"

#include "ChannelManager.h"
#include "SeManager.h"

namespace mm2hack::apps::audio
{
    SeManager::SeManager(ChannelManager& bgmChannels, int seChannelCount)
        : _bgmChannels(bgmChannels), _seChannels(seChannelCount)
    {
        _bgmVolumeBackup.resize(_bgmChannels.GetChannelCount(), 255);
    }

    bool SeManager::LoadSe(const std::wstring& name, const std::wstring& filepath)
    {
        if (name.empty() || filepath.empty()) return false;
        _seData[name] = { filepath, 255 };
        return true;
    }

    void SeManager::PlaySe(const std::wstring& name, int volume)
    {
        auto it = _seData.find(name);
        if (it == _seData.end()) return;

        int channelIndex = -1;
        for (int i = 0; i < _seChannels.GetChannelCount(); ++i)
        {
            if (!_seChannels.IsPlaying(i))
            {
                channelIndex = i;
                break;
            }
        }

        // If no channel is available, stop the oldest SE and reuse it.
        if (channelIndex == -1)
        {
            channelIndex = 0;
            _seChannels.Stop(channelIndex);
        }

        _seChannels.Load(channelIndex, it->second.filepath);
        _seChannels.SetVolume(channelIndex, std::clamp(volume, 0, 255));
        _seChannels.Play(channelIndex, false);

        // Mute BGM channels.
        for (int i = 0; i < _bgmChannels.GetChannelCount(); ++i)
        {
            if (_bgmChannels.GetVolume(i) > 0)
            {
                _bgmVolumeBackup[i] = _bgmChannels.GetVolume(i);
                _bgmChannels.SetVolume(i, 0);
            }
        }
    }

    void SeManager::StopAll()
    {
        _seChannels.StopAll();
    }

    void SeManager::Update()
    {
        _seChannels.Update();

        // If all SEs have stopped, restore BGM volume.
        bool anyPlaying = false;
        for (int i = 0; i < _seChannels.GetChannelCount(); ++i)
        {
            if (_seChannels.IsPlaying(i))
            {
                anyPlaying = true;
                break;
            }
        }

        if (!anyPlaying)
        {
            for (int i = 0; i < _bgmChannels.GetChannelCount(); ++i)
            {
                _bgmChannels.SetVolume(i, _bgmVolumeBackup[i]);
            }
        }
    }

    void SeManager::SetMasterVolume(int volume)
    {
        _masterVolume = std::clamp(volume, 0, 255);
        _seChannels.SetAllVolumes(_masterVolume);
    }
}