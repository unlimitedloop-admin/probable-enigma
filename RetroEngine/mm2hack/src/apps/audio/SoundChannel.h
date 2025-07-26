#pragma once

#include "ISoundChannel.h"

#include <string>

namespace mm2hack::apps::audio
{
    class SoundChannel : public ISoundChannel
    {
    public:
        SoundChannel();
        ~SoundChannel() override;

        // Load a sound file
        bool Load(const std::wstring& filepath) override;
        // Play the sound
        void Play(bool loop = false) override;
        // Stop the sound
        void Stop() override;
        // Set volume (0-255)
        void SetVolume(int volume) override;
        // Get current volume
        int  GetVolume() const override;
        // Check if the sound is playing
        bool IsPlaying() const override;

        // Start fade (change to targetVolume over durationFrames)
        void StartFade(int targetVolume, int durationFrames) override;
        // Update every frame (for fade processing, etc.)
        void Update() override;

    private:
        int _handle = -1;
        int _volume = 255;

        // Additional fade control variables
        int _fade_target = 255;
        int _fade_step = 0;
        int _fade_frames_remaining = 0;
    };
}
