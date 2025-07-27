#include "pch.h"

#include "SoundTest.h"

#include "apps/audio/AudioInitializer.h"
#include "apps/audio/BgmManager.h"

namespace mm2hack::apps::scenes
{
    bool SoundTest::Initialize()
    {
        // Initialize audio systems here
        // For example, load audio configurations, initialize BGM and SE managers, etc.
        if (!audio::AudioInitializer::InitializeAudio(L"src\\resources\\exams\\audio\\json\\audio_config.json", _bgmManager, _seManager, _bgmChannels))
        {
            return false; // Failed to initialize audio systems
        }

        _bgmManager.Play(L"sample2");   // Play a sample BGM
        return true;
    }

    void SoundTest::Update()
    {
        // Update audio systems, handle input for playing/stopping sounds, etc.
        _bgmManager.Update();
    }

    void SoundTest::Draw()
    {
        // Draw any UI elements related to the sound test
    }

    void SoundTest::Finalize()
    {
        // Clean up audio systems and resources
        _bgmManager.Stop();
    }
}