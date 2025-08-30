#include "pch.h"

#include "JoystickManager.h"

#include <cstdint>
#include <optional>
#include "C16ButtonState.h"
#include "DirectInputProvider.h"
#include "IInputProvider.h"
#include "InputFrame.h"
#include "KeyBinding.h"
#include "KeyboardInputProvider.h"
#include "KeyToken.h"
#include "RawInputEvent.h"
#include "XInputProvider.h"

namespace mm2hack::input
{
    JoystickManager::JoystickManager()
    {
        if (CheckXInputAvailable())
        {
            _provider = std::make_unique<XInputProvider>(_binding);
            _activeKind = Device::XInput;
        }
        else if (CheckDirectInputAvailable())
        {
            _provider = std::make_unique<DirectInputProvider>(_binding);
            _activeKind = Device::DirectInput;
        }
        else
        {
            _provider = std::make_unique<KeyboardInputProvider>(_binding);
            _activeKind = Device::Keyboard;
        }

        _binding.SetDefaultBindingSCon(_activeKind);
    }

    bool JoystickManager::Update()
    {
        return _provider->Update(_button_state);
    }

    const InputFrame& JoystickManager::GetButtonState(size_t index) const
    {
        return _button_state.GetState(index);
    }

    const C16ButtonState& JoystickManager::GetAllStates() const
    {
        return _button_state;
    }

    bool JoystickManager::IsEnableInputDevice() const
    {
        return _provider != nullptr;
    }

    std::optional<RawInputEvent> JoystickManager::PollFirstRawChange(float deadzone, std::optional<Device> only) noexcept
    {
        const Device kind = only.value_or(_activeKind);

        // For keyboard
        if (kind == Device::Keyboard)
        {
            for (int vk = 0; vk < 256; ++vk)
            {
                if (CheckHitKey(vk))
                {
                    return RawInputEvent{ RawDevice::Keyboard, RawKind::Key, static_cast<uint8_t>(vk), false, 1.0f };
                }
            }
            return std::nullopt;
        }

        if (kind == Device::XInput)
        {
            XINPUT_STATE st{};
            if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &st) != 0)
            {
                return std::nullopt;
            }
            // Buttons
            for (uint8_t bi = 0; bi < 16; ++bi)
            {
                if (st.Buttons[bi])
                {
                    return RawInputEvent{ RawDevice::XInput, RawKind::Button, bi, false, 1.0f };
                }
            }
            // Triggers
            const float thr = deadzone;
            if (st.LeftTrigger / 255.0f >= thr) return RawInputEvent{ RawDevice::XInput, RawKind::Trigger, 0, false, st.LeftTrigger / 255.0f };
            if (st.RightTrigger / 255.0f >= thr) return RawInputEvent{ RawDevice::XInput, RawKind::Trigger, 1, false, st.RightTrigger / 255.0f };
            // Axes
            const int athr = static_cast<int>(thr * 32767.0f + 0.5f);
            struct A { int v; uint8_t idx; };
            for (A a : { A{ st.ThumbLX,0 }, A{ st.ThumbLY,1 }, A{ st.ThumbRX,2 }, A{ st.ThumbRY,3 } })
            {
                if (a.v >= athr) return RawInputEvent{ RawDevice::XInput, RawKind::Axis, a.idx, false, static_cast<float>(a.v) / 32767.0f };
                if (a.v <= -athr) return RawInputEvent{ RawDevice::XInput, RawKind::Axis, a.idx, true,  static_cast<float>(-a.v) / 32767.0f };
            }
            return std::nullopt;
        }

        if (kind == Device::DirectInput)
        {
            DINPUT_JOYSTATE st{};
            if (GetJoypadDirectInputState(DX_INPUT_KEY_PAD1, &st) != 0) return std::nullopt;

            using clock = std::chrono::steady_clock;
            struct P { float deadzone = 0.25f, pressThr = 0.60f; int holdMs = 120, settleMs = 120; };
            static P p{};

            struct Ctx
            {
                bool measuring = true, ready = false;
                float cx = 0.f, cy = 0.f; clock::time_point t0{};
                bool holding = false; uint8_t pickedAxis = 0; bool pickedNeg = false; clock::time_point holdStart{};
            };
            static Ctx xy{}, rxry{}, zrz{}, s01{};

            auto normSigned = [](long v)->float
                {
                    const float denom = (std::abs(v) > 2000) ? 32767.0f : 1000.0f;
                    float f = static_cast<float>(v) / denom;
                    return std::clamp(f, -1.0f, 1.0f);
                };

            auto capturePair = [&](long vx_raw, long vy_raw, Ctx& ctx, uint8_t codeX, uint8_t codeY)
                -> std::optional<RawInputEvent>
                {
                    const float rx = normSigned(vx_raw), ry = normSigned(vy_raw);
                    if (ctx.measuring)
                    {
                        if (ctx.t0.time_since_epoch().count() == 0) ctx.t0 = clock::now();
                        ctx.cx = ctx.cx * 0.85f + rx * 0.15f; ctx.cy = ctx.cy * 0.85f + ry * 0.15f;
                        if (clock::now() - ctx.t0 >= std::chrono::milliseconds(p.settleMs)) { ctx.measuring = false; ctx.ready = true; }
                        return std::nullopt;
                    }
                    if (!ctx.ready) { ctx.measuring = true; ctx.t0 = clock::now(); return std::nullopt; }

                    const float x = rx - ctx.cx, y = ry - ctx.cy;
                    auto absv = [](float v) { return v >= 0 ? v : -v; };
                    const float ax = (absv(x) >= p.deadzone) ? x : 0.f;
                    const float ay = (absv(y) >= p.deadzone) ? y : 0.f;
                    if (ax == 0.f && ay == 0.f) { ctx.holding = false; return std::nullopt; }

                    const uint8_t axis = (absv(ax) >= absv(ay)) ? 0u : 1u;
                    const bool neg = (axis == 0u) ? (ax < 0) : (ay < 0);
                    const float val = absv((axis == 0u) ? ax : ay);

                    if (val >= p.pressThr)
                    {
                        if (!ctx.holding || axis != ctx.pickedAxis || neg != ctx.pickedNeg)
                        {
                            ctx.holding = true; ctx.pickedAxis = axis; ctx.pickedNeg = neg; ctx.holdStart = clock::now();
                        }
                        if (clock::now() - ctx.holdStart >= std::chrono::milliseconds(p.holdMs))
                        {
                            const uint8_t code = (axis == 0u) ? codeX : codeY; // 0=X,1=Y  /  3=Rx,4=Ry
                            return RawInputEvent{ RawDevice::DirectInput, RawKind::Axis, code, neg, val };
                        }
                    }
                    else { ctx.holding = false; }
                    return std::nullopt;
                };

            auto tryPair = [&](long vx, long vy, Ctx& ctx, uint8_t codeX, uint8_t codeY)
                -> std::optional<RawInputEvent>
                {
                    return capturePair(vx, vy, ctx, codeX, codeY); // 既存の capturePair をそのまま使う
                };

            //    左狙い＝XY→RxRy→ZRz→S0S1 の順で、最初に確定したものを返す
            if (_diCaptureGroup != AxisGroup::Right)
            {
                if (auto ev = tryPair(st.X, st.Y, xy,   /*codeX=*/0, /*codeY=*/1)) return ev; // X/Y
                if (auto ev = tryPair(st.Rx, st.Ry, rxry, /*codeX=*/3, /*codeY=*/4)) return ev; // Rx/Ry
                if (auto ev = tryPair(st.Z, st.Rz, zrz,  /*codeX=*/2, /*codeY=*/5)) return ev; // Z/Rz
                if (auto ev = tryPair(st.Slider[0], st.Slider[1], s01, /*codeX=*/6, /*codeY=*/7)) return ev; // S0/S1
            }

            //    右狙い＝RxRy→ZRz→XY→S0S1 の順
            if (_diCaptureGroup != AxisGroup::Left)
            {
                if (auto ev = tryPair(st.Rx, st.Ry, rxry, /*codeX=*/3, /*codeY=*/4)) return ev; // Rx/Ry
                if (auto ev = tryPair(st.Z, st.Rz, zrz,  /*codeX=*/2, /*codeY=*/5)) return ev; // Z/Rz
                if (auto ev = tryPair(st.X, st.Y, xy,   /*codeX=*/0, /*codeY=*/1)) return ev; // X/Y
                if (auto ev = tryPair(st.Slider[0], st.Slider[1], s01, /*codeX=*/6, /*codeY=*/7)) return ev; // S0/S1
            }

            // 次に POV（任意）
            const unsigned int pov = st.POV[0];
            if (pov != 0xFFFFFFFFu)
            {
                const bool up = (pov == 0u || pov == 4500u || pov == 31500u);
                const bool right = (pov == 9000u || pov == 4500u || pov == 13500u);
                const bool down = (pov == 18000u || pov == 13500u || pov == 22500u);
                const bool left = (pov == 27000u || pov == 22500u || pov == 31500u);
                if (up)    return RawInputEvent{ RawDevice::DirectInput, RawKind::Axis, 1, true,  1.0f };
                if (right) return RawInputEvent{ RawDevice::DirectInput, RawKind::Axis, 0, false, 1.0f };
                if (down)  return RawInputEvent{ RawDevice::DirectInput, RawKind::Axis, 1, false, 1.0f };
                if (left)  return RawInputEvent{ RawDevice::DirectInput, RawKind::Axis, 0, true,  1.0f };
            }

            // 最後にボタン等（必要なら）
            for (uint8_t bi = 0; bi < 32; ++bi) if (st.Buttons[bi])
                return RawInputEvent{ RawDevice::DirectInput, RawKind::Button, bi, false, 1.0f };

            return std::nullopt;
        }

        return std::nullopt;
    }

    bool JoystickManager::CheckXInputAvailable()
    {
        XINPUT_STATE state{};
        if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }
        return true;
    }

    bool JoystickManager::CheckDirectInputAvailable()
    {
        DINPUT_JOYSTATE state{};
        if (GetJoypadDirectInputState(DX_INPUT_KEY_PAD1, &state) != 0)
        {
            return false;
        }

        if (state.POV[0] >= 0)
        {
            return true;
        }

        for (int i = 0; i < 32; ++i)
        {
            if (state.Buttons[i] != 0)
            {
                return true;
            }
        }

        return false;
    }
}