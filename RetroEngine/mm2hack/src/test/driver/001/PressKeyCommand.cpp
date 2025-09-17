#include "pch.h"

#include "PresskeyCommand.h"

#include "apps/deal/GameContext.h"
#include "core/assembly/InputTypes.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    bool PressKeyCommand::Initialize()
    {
        return true;
    }

    void PressKeyCommand::Update()
    {
        auto& joystick = apps::deal::GameContext::GetInstance().Input();
        joystick.UpdateJoystick();
    }

    void PressKeyCommand::RenderWorld()
    {
        using namespace core::assembly;

        auto& joystick = apps::deal::GameContext::GetInstance().Input();
        // Show the state of each button.
        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const auto& frame = joystick.PressedFrames(static_cast<Key16>(i));

            std::wstring line = L"INPUT KEY " + std::to_wstring(i) + L" : " + std::to_wstring(frame);
            DrawString(10, 10 + static_cast<int>(i) * 14, line.c_str(), GetColor(255, 255, 255));
        }
    }

    void PressKeyCommand::Finalize()
    {
    }
}