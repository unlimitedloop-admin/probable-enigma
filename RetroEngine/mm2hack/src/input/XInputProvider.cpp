#include "pch.h"

#include "XInputProvider.h"

#include "C16ButtonState.h"
#include "Jpbtn.h"
#include "KeyToken.h"
#include "xi/XInputToken.h"

namespace mm2hack::input
{
    bool XInputProvider::Update(C16ButtonState& out_state)
    {
        XINPUT_STATE state{};
        if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }

        auto btn = [&](uint8_t i) { return state.Buttons[i] != 0; };
        auto trg = [&](bool left, float thr01)
            {
                const int raw = left ? state.LeftTrigger : state.RightTrigger; // 0..255
                const int thr = static_cast<int>(thr01 * 255.0f + 0.5f);
                return raw >= thr;
            };
        auto axis = [&](uint8_t idx, bool neg, float thr01)
            {
                const int raw = (idx == xi::LX ? state.ThumbLX : idx == xi::LY ? state.ThumbLY : idx == xi::RX ? state.ThumbRX : state.ThumbRY); // -32768..32767
                const int thr = static_cast<int>(thr01 * 32767.0f + 0.5f);
                return neg ? (raw <= -thr) : (raw >= thr);
            };

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const auto token = _binding.GetBindingSCon(static_cast<JPBTN>(i));
            bool pressed = false;

            if (IsUnboundToken(token))
            {
                pressed = false;
            }
            else if (xi::IsBtn(token))
            {
                pressed = btn(xi::Code(token)); // Buttons[] index
            }
            else if (xi::IsTrig(token))
            {
                const uint8_t which = xi::Code(token); // 0=LT,1=RT
                pressed = trg(which == xi::LT, xi::Thr01(token));
            }
            else if (xi::IsAxis(token))
            {
                pressed = axis(xi::Code(token), xi::IsNeg(token), xi::Thr01(token));
            }

            out_state.UpdateButton(i, pressed);
        }
        return true;
    }
}