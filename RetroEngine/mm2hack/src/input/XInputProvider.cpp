#include "XInputProvider.h"

#include <DxLib.h>
#include "C16ButtonState.h"
#include "Jpbtn.h"

namespace mm2hack::input
{
    bool XInputProvider::Update(C16ButtonState& out_state)
    {
        XINPUT_STATE state{};
        if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }

        constexpr int xinput_btn[JPBTN_COUNT] = {
            XINPUT_BUTTON_DPAD_UP,
            XINPUT_BUTTON_DPAD_DOWN,
            XINPUT_BUTTON_DPAD_LEFT,
            XINPUT_BUTTON_DPAD_RIGHT,
            XINPUT_BUTTON_A,
            XINPUT_BUTTON_B,
            XINPUT_BUTTON_X,
            XINPUT_BUTTON_Y,
            XINPUT_BUTTON_START,
            XINPUT_BUTTON_BACK,
            XINPUT_BUTTON_LEFT_SHOULDER,
            XINPUT_BUTTON_RIGHT_SHOULDER,
            -1, // LT
            -1, // RT
            XINPUT_BUTTON_LEFT_THUMB,
            XINPUT_BUTTON_RIGHT_THUMB
        };

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            bool pressed = false;

            if (i == static_cast<size_t>(JPBTN::LTRIGGER))
            {
                pressed = state.LeftTrigger > 30;
            }
            else if (i == static_cast<size_t>(JPBTN::RTRIGGER))
            {
                pressed = state.RightTrigger > 30;
            }
            else
            {
                int btn_index = xinput_btn[i];
                if (btn_index >= 0)
                {
                    pressed = state.Buttons[btn_index] != 0;
                }
            }
            out_state.UpdateButton(i, pressed);
        }

        return true;
    }
}