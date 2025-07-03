#include "DirectInputProvider.h"

#include <DxLib.h>
#include "C16ButtonState.h"
#include "Jpbtn.h"

namespace mm2hack::input
{
    bool DirectInputProvider::Update(C16ButtonState& out_state)
    {
        DINPUT_JOYSTATE state{};
        if (DxLib::GetJoypadDirectInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }

        // Here are some typical examples, though they may vary depending on the device.
        constexpr int directinput_btn[JPBTN_COUNT] = {
            -1, // UP: for POV
            -1, // DOWN
            -1, // LEFT
            -1, // RIGHT
            0,  // A
            1,  // B
            3,  // X
            4,  // Y
            11, // START
            10, // BACK
            6,  // LB
            7,  // RB
            8,  // LT (May not be present on some devices)
            9,  // RT (ditto)
            13, // LTHUMB
            14  // RTHUMB
        };

        // Convert POV (directional pad) angle to UP/DOWN/LEFT/RIGHT
        const unsigned int pov = state.POV[0];
        const bool has_pov = (pov != 0xFFFFFFFF);
        const bool up    = has_pov && (pov == 0     || pov == 4500  || pov == 31500);
        const bool right = has_pov && (pov == 9000  || pov == 4500  || pov == 13500);
        const bool down  = has_pov && (pov == 18000 || pov == 13500 || pov == 22500);
        const bool left  = has_pov && (pov == 27000 || pov == 22500 || pov == 31500);

        out_state.UpdateButton(static_cast<int>(JPBTN::UP), up);
        out_state.UpdateButton(static_cast<int>(JPBTN::DOWN), down);
        out_state.UpdateButton(static_cast<int>(JPBTN::LEFT), left);
        out_state.UpdateButton(static_cast<int>(JPBTN::RIGHT), right);

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            if (i <= static_cast<size_t>(JPBTN::RIGHT))
            {
                continue;   // The procedure is completed in POV.
            }

            const int btn_index = directinput_btn[i];
            const bool pressed = (btn_index >= 0 && btn_index < 32) ? (state.Buttons[btn_index] != 0) : false;

            out_state.UpdateButton(static_cast<int>(i), pressed);
        }

        return true;
    }
}