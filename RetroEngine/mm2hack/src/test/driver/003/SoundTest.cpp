#include "pch.h"

#include "SoundTest.h"

#include "apps/deal/GameContext.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    bool SoundTest::Initialize()
    {
        // Initialize audio systems here
        auto& audio = deal::GameContext::GetInstance().GetResourceManager().GetAudioManager();
        return audio.Initialize(L"src\\resources\\exams\\audio\\json\\audio_config.json");
    }

    void SoundTest::Update()
    {
        // Update audio systems, handle input for playing/stopping sounds, etc.
        auto& joystick = deal::GameContext::GetInstance().GetJoystickManager();
        joystick.Update();

        auto& audio = deal::GameContext::GetInstance().GetResourceManager().GetAudioManager();

        if (joystick.GetButtonState(JPBTN::START).pressed_frame == 1)
        {
            if (_isPlayThisTrack)
            {
                audio.StopBgm();
                _isPlayThisTrack = false;
            }
            else
            {
                audio.PlayBgm(L"sample1");
                _isPlayThisTrack = true;
            }
        }

        if (joystick.GetButtonState(JPBTN::A).pressed_frame == 1)
        {
            audio.PlaySe(L"1up");
        }

        if (joystick.GetButtonState(JPBTN::B).pressed_frame == 1)
        {
            audio.PlaySe(L"sonic_boom");
        }

        // Example: Check for input to play a specific track
        if (_isPlayThisTrack)
        {
            audio.Update();
        }
    }

    void SoundTest::Draw()
    {
        // Draw any UI elements related to the sound test
    }

    void SoundTest::Finalize()
    {
        // Clean up audio systems and resources
    }
}