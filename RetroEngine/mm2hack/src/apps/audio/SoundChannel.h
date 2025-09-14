//==============================================================================
// 
//  Project: mm2hack
//  SoundChannel.h
// 
//  Sound channel implementation for audio playback in applications.
// 
//==============================================================================
#pragma once

#include "ISoundChannel.h"

#include <string>
#include <Windows.h>
#include "config/SystemConfig.h"

namespace mm2hack::apps::audio
{
    enum class SoundChip { APU, VRC6, MMC5, N163 };

    struct ChannelInfo
    {
        SoundChip chip;
        int index;
    };

    // Sound channel implementation
    class SoundChannel : public ISoundChannel
    {
    public:
        SoundChannel();
        ~SoundChannel() override;

        // Load a sound file
        bool Load(const std::wstring& filepath) override;
        // Play the sound
        void Play(bool loop) override;
        // Stop the sound
        void Stop() override;
        // Pause the sound
        void Pause() override;
        // Resume the sound
        void Resume(bool loop) override;
        // Set volume (0-255)
        void SetVolume(int volume) override;
        // Get current volume
        int  GetVolume() const override;
        // Check if the sound is playing
        bool IsPlaying() const override;
        // Get the current position in the sound (in frames)
        LONGLONG GetPosition() const;
        // Set the current position in the sound (in frames)
        void SetPosition(LONGLONG pos) const;

        // Start fade (change to targetVolume over durationFrames)
        void StartFade(int targetVolume, int durationFrames) override;
        // Update every frame (for fade processing, etc.)
        void Update() override;

        // Set and get sound chip info
        void SetChipInfo(SoundChip chip, int index);
        ChannelInfo GetChipInfo() const;

        // Get the handle of the sound channel (for external use)
        int GetNativeHandle() const override { return _handle; }

    private:
        const int MAX_VOLUME = config::SystemConfig::kAudioMaxVolume;

        int _handle = -1;
        int _volume = MAX_VOLUME;
        LONGLONG _pausedPos = 0;
        bool _wasPaused = false;

        // Additional fade control variables
        int _fade_target = MAX_VOLUME;
        int _fade_step = 0;
        int _fade_frames_remaining = 0;

        SoundChip _chip = SoundChip::APU;
        int _chipIndex = 0;
    };
}