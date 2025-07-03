#include "PresskeyCommand.h"

#include <DxLib.h>
#include <string>
#include "input/JoystickSingleton.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    bool PressKeyCommand::Initialize()
    {
        using joystick = input::JoystickSingleton;
        joystick::Initialize();
        return joystick::Instance().IsEnableInputDevice();
    }

    void PressKeyCommand::Update()
    {
        auto& joystick = input::JoystickSingleton::Instance();

        // 入力更新
        joystick.Update();

        // 16キー分を表示
        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const auto& frame = joystick.GetButtonState(i);

            std::wstring line = L"INPUT KEY " + std::to_wstring(i) + L" : " + std::to_wstring(frame.pressed_frame);
            DrawString(10, 10 + static_cast<int>(i) * 14, line.c_str(), GetColor(255, 255, 255));
        }
    }

    void PressKeyCommand::Finalize()
    {
        input::JoystickSingleton::Finalize();
    }
}