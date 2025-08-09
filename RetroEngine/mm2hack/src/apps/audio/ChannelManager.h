//==============================================================================
// 
//  Project: mm2hack
//  ChannelManager.h
// 
//  Manages multiple audio channels for playback.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "ISoundChannel.h"
#include "SoundChannel.h"

namespace mm2hack::apps::audio
{
    // Audio channel manager that handles multiple sound channels
    class ChannelManager
    {
    public:
        ChannelManager(int defaultChannels = 5);    // NES default 5 channels
        ~ChannelManager() = default;

        // Add a new sound channel, returns the index of the new channel
        int AddChannel(std::unique_ptr<ISoundChannel> channel = std::make_unique<SoundChannel>());
        // Load a sound file into the specified channel
        bool Load(int channelIndex, const std::wstring& filepath);
        // Play, stop, set volume, get volume, check if playing
        void Play(int channelIndex, bool loop = false);
        // Stop the sound in the specified channel
        void Stop(int channelIndex);
        // Set volume for the specified channel (0-255)
        void SetVolume(int channelIndex, int volume);
        // Get current volume of the specified channel
        int GetVolume(int channelIndex) const;
        // Check if the specified channel is currently playing
        bool IsPlaying(int channelIndex) const;

        // Start fade effect for the specified channel
        void StartFade(int channelIndex, int targetVolume, int durationFrames);

        // Stop all channels
        void StopAll();
        // Pause all channels (saves current position)
        void PauseAll();
        // Resume all channels (resumes from saved position)
        void ResumeAll(bool loop);
        // Set volume for all channels (0-255), when use pause/resume, volume is not changed
        void SetAllVolumes(int volume);

        // Update all channels (for fade processing, etc.)
        void Update();

        // Get the number of channels managed
        int GetChannelCount() const { return static_cast<int>(_channels.size()); }
        // Ensure the channel count is at least 'count', adding empty channels if necessary
        void EnsureChannelCount(int count);

        // Get the handle of the specified channel (for external use)
        int GetHandle(int channelIndex) const;

        void Clear();

    private:
        std::vector<std::unique_ptr<ISoundChannel>> _channels;      // List of sound channels
    };
}