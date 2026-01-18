//==============================================================================
// 
//  Project: mm2hack
//  ISoundChannel.h
// 
//  Multi-channel sound interface for audio playback in applications.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::apps::systems::audio
{
    // Interface for sound channel management
    class ISoundChannel
    {
    public:
        virtual ~ISoundChannel() = default;

        // Load a sound file
        virtual bool Load(const std::wstring& filepath) = 0;
        // Play the sound
        virtual void Play(bool loop = false) = 0;
        // Stop the sound
        virtual void Stop() = 0;
        // Pause the sound
        virtual void Pause() = 0;
        // Resume the sound
        virtual void Resume(bool loop) = 0;
        // Set volume (0-255)
        virtual void SetVolume(int volume) = 0;
        // Get current volume
        virtual int GetVolume() const = 0;
        // Check if the sound is playing
        virtual bool IsPlaying() const = 0;
        // Start fade (change to targetVolume over durationFrames)
        virtual void StartFade(int targetVolume, int durationFrames) = 0;
        // Update every frame (for fade processing, etc.)
        virtual void Update() = 0;
        // Get the handle of the sound channel (for external use)
        virtual int GetNativeHandle() const = 0;
    };
}