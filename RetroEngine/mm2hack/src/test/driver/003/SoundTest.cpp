#include "pch.h"

#include "SoundTest.h"

#include "apps/runtime/GameContext.h"
#include "core/assembly/InputTypes.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    bool SoundTest::Initialize()
    {
        // Initialize audio systems here
        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();
        return audio.Initialize(std::wstring(L"assets\\_exams\\audio\\json\\audio_config.json"));
    }

    void SoundTest::Update()
    {
        using namespace core::assembly;
        // Update audio systems, handle input for playing/stopping sounds, etc.
        auto& input = runtime::GameContext::GetInstance().Input();
        auto& audio = runtime::GameContext::GetInstance().GetResourceManager().GetAudioManager();

        if (input.PressedFrames(Key16::START) == 1)
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

        if (input.PressedFrames(Key16::A) == 1)
        {
            audio.PlaySe(L"1up");
        }

        if (input.PressedFrames(Key16::B) == 1)
        {
            audio.PlaySe(L"icarus_block");
        }

        // Ex. Check for input to play a specific track
        if (_isPlayThisTrack)
        {
            audio.Update();
        }
    }

    void SoundTest::RenderWorld()
    {
        // Draw any UI elements related to the sound test
    }

    void SoundTest::RenderOverlay()
    {
        // Draw any overlay elements, such as debug information or instructions
        DxLib::DrawString(20, 36, L"Press START to toggle sample BGM", DxLib::GetColor(255, 255, 255));
        DxLib::DrawString(20, 56, L"Press A to play '1up' sound effect", DxLib::GetColor(255, 255, 255));
        DxLib::DrawString(20, 76, L"Press B to play 'AppearingBlock' sound effect", DxLib::GetColor(255, 255, 255));
        DxLib::DrawString(20, 96, L"The BGM channel will be muted while a sound effect", DxLib::GetColor(255, 255, 255));
        DxLib::DrawString(20, 116, L"that overlaps with the BGM is being played.", DxLib::GetColor(255, 255, 255));
    }

    void SoundTest::Finalize()
    {
        // Clean up audio systems and resources
    }
}