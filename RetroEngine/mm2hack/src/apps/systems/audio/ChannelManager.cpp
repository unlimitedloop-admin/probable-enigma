#include "pch.h"

#include "ChannelManager.h"

#include "ISoundChannel.h"
//#include "utils/output_debug.h"

namespace mm2hack::apps::systems::audio
{
    ChannelManager::ChannelManager(int defaultChannels)
    {
        for (int i = 0; i < defaultChannels; ++i)
        {
            AddChannel();
        }
    }

    int ChannelManager::AddChannel(std::unique_ptr<ISoundChannel> channel)
    {
        _channels.push_back(std::move(channel));
        int index = static_cast<int>(_channels.size() - 1);
        int total = static_cast<int>(_channels.size());
        //utils::debug_log(L"[ChannelManager] AddChannel: index={}, total={}", index, total);
        return index;
    }

    bool ChannelManager::Load(int channelIndex, const std::wstring& filepath)
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return false;
        return _channels[channelIndex]->Load(filepath);
    }

    void ChannelManager::Play(int channelIndex, bool loop)
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return;
        _channels[channelIndex]->Play(loop);
    }

    void ChannelManager::Stop(int channelIndex)
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return;
        _channels[channelIndex]->Stop();
    }

    void ChannelManager::SetVolume(int channelIndex, int volume)
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return;
        _channels[channelIndex]->SetVolume(volume);
    }

    int ChannelManager::GetVolume(int channelIndex) const
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return 0;
        return _channels[channelIndex]->GetVolume();
    }

    bool ChannelManager::IsPlaying(int channelIndex) const
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return false;
        return _channels[channelIndex]->IsPlaying();
    }

    void ChannelManager::StartFade(int channelIndex, int targetVolume, int durationFrames)
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return;
        _channels[channelIndex]->StartFade(targetVolume, durationFrames);
    }

    void ChannelManager::StopAll()
    {
        for (auto& ch : _channels)
        {
            ch->Stop();
        }
    }

    void ChannelManager::PauseAll()
    {
        for (auto& ch : _channels)
        {
            ch->Pause();
        }
    }

    void ChannelManager::ResumeAll(bool loop)
    {
        for (auto& ch : _channels)
        {
            ch->Resume(loop);
        }
    }

    void ChannelManager::SetAllVolumes(int volume)
    {
        for (auto& ch : _channels)
        {
            ch->SetVolume(volume);
        }
    }

    void ChannelManager::Update()
    {
        for (auto& ch : _channels)
        {
            ch->Update();
        }
    }

    void ChannelManager::EnsureChannelCount(int count)
    {
        while (GetChannelCount() < count)
        {
            AddChannel();
        }
    }

    int ChannelManager::GetHandle(int channelIndex) const
    {
        if (channelIndex < 0 || channelIndex >= GetChannelCount()) return -1;
        return _channels[channelIndex]->GetNativeHandle();
    }

    void ChannelManager::Clear()
    {
        for (auto& ch : _channels)
        {
            ch.reset();     // Stop any playing sound before clearing
        }
        _channels.clear();
    }
}