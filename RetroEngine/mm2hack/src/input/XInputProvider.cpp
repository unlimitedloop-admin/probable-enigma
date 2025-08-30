#include "pch.h"

#include "XInputProvider.h"

#include <cstdint>
#include "C16ButtonState.h"
#include "Jpbtn.h"
#include "KeyToken.h"

namespace mm2hack::input
{
    bool XInputProvider::Update(C16ButtonState& out_state)
    {
        XINPUT_STATE state{};
        if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }

        // Buttons[] の想定：ユーザー既存コードに合わせインデックス式
        auto get_button_pressed = [&](uint8_t idx)->bool
            {
                // 範囲チェックは適宜（ここでは簡潔に）
                return state.Buttons[idx] != 0;
            };

        auto get_trigger_pressed = [&](bool left, float thr01)->bool
            {
                const int raw = left ? state.LeftTrigger : state.RightTrigger; // 0..255 想定
                return raw >= static_cast<int>(thr01 * 255.0f + 0.5f);
            };

        auto get_axis_pressed = [&](uint8_t axisIdx, bool negative, float thr01)->bool
            {
                // -32768..32767 を想定（DxLibのXINPUT_STATEに準拠）
                int raw = 0;
                switch (axisIdx)
                {
                case XI_LX: raw = state.ThumbLX; break;
                case XI_LY: raw = state.ThumbLY; break;
                case XI_RX: raw = state.ThumbRX; break;
                case XI_RY: raw = state.ThumbRY; break;
                default: break;
                }
                const int thr = static_cast<int>(thr01 * 32767.0f + 0.5f);
                return negative ? (raw <= -thr) : (raw >= thr);
            };

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const auto token = _binding.GetBindingSCon(static_cast<JPBTN>(i));
            bool pressed = false;

            if (token != 0xFFFFu)
            {
                // 最小仕様：ボタンのみ（token は Buttons[] の index）
                const uint8_t btn_idx = static_cast<uint8_t>(token & 0xFFu);
                // 範囲チェックを追加
                if (btn_idx < sizeof(state.Buttons) / sizeof(state.Buttons[0]))
                {
                    pressed = (state.Buttons[btn_idx] != 0);
                }
                else
                {
                    pressed = false;
                }
            }

            out_state.UpdateButton(i, pressed);
        }

        return true;
    }
}