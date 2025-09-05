#include "pch.h"

#include "DirectInputProvider.h"

#include <cstdint>
#include "C16ButtonState.h"
#include "di/DirectInputToken.h"
#include "Jpbtn.h"
#include "KeyToken.h"

namespace mm2hack::input
{
    bool DirectInputProvider::Update(C16ButtonState& out_state)
    {
        using namespace di;
        DINPUT_JOYSTATE state{};
        if (GetJoypadDirectInputState(DX_INPUT_KEY_PAD1, &state) != 0) return false;

        const unsigned int pov = state.POV[0];
        const bool hasPOV = (pov != 0xFFFFFFFFu);
        const bool povU = hasPOV && (pov == 0u || pov == 4500u || pov == 31500u);
        const bool povR = hasPOV && (pov == 9000u || pov == 4500u || pov == 13500u);
        const bool povD = hasPOV && (pov == 18000u || pov == 13500u || pov == 22500u);
        const bool povL = hasPOV && (pov == 27000u || pov == 22500u || pov == 31500u);

        auto norm01 = [](long v)->float
            {
                const long a = (v >= 0) ? v : -v;
                const float denom = (a > 2000) ? 32767.0f : 1000.0f;
                return std::min(1.0f, static_cast<float>(a) / denom);
            };
        auto axisPressed = [&](uint8_t idx, bool neg, float thr01)->bool
            {
                long raw = 0;
                switch (idx)
                {
                case AX_X: raw = state.X; break;   case AX_Y: raw = state.Y; break;
                case AX_Z: raw = state.Z; break;   case AX_Rx: raw = state.Rx; break;
                case AX_Ry: raw = state.Ry; break; case AX_Rz: raw = state.Rz; break;
                case AX_S0: raw = state.Slider[0]; break; case AX_S1: raw = state.Slider[1]; break;
                default: return false;
                }
                const float v = norm01(raw);
                if (v < thr01) return false;
                return neg ? (raw < 0) : (raw > 0);
            };

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const auto token = _binding.GetBindingSCon(static_cast<JPBTN>(i));
            bool pressed = false;

            if (!IsUnboundToken(token))
            {
                if (IsBtn(token))
                {
                    const uint8_t idx = Code(token);
                    pressed = (idx < 32) ? (state.Buttons[idx] != 0) : false;
                }
                else if (IsPOV(token))
                {
                    switch (Code(token))
                    {
                    case POV_U: pressed = povU; break;
                    case POV_R: pressed = povR; break;
                    case POV_D: pressed = povD; break;
                    case POV_L: pressed = povL; break;
                    }
                }
                else if (IsAxis(token))
                {
                    pressed = axisPressed(Code(token), IsNeg(token), Thr01(token));
                    if (!pressed && (Code(token) == AX_X || Code(token) == AX_Y))
                    {
                        pressed = (Code(token) == AX_X) ? (IsNeg(token) ? povL : povR)
                            : (IsNeg(token) ? povU : povD);
                    }
                }
            }
            else
            {
                // old binding (0xFFFF) means unbound [no input]
            }

            out_state.UpdateButton(i, pressed);
        }
        return true;
    }
}