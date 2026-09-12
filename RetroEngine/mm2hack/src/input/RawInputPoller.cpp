#include "pch.h"

#include "RawInputPoller.h"

#include <cstdlib>
#include "JoystickManager.h"
#include "KeyToken.h"
#include "RawInputEvent.h"

namespace mm2hack::input
{
    std::optional<RawInputEvent> RawInputPoller::PollFirstRawChange(
        float deadzone,
        std::optional<Device> only,
        Device activeKind,
        AxisGroup diCaptureGroup
    )
    {
        const Device kind = only.value_or(activeKind);

        if (kind == Device::Keyboard)
            return pollKeyboardRawChange_();

        if (kind == Device::XInput)
            return pollXInputRawChange_(deadzone);

        if (kind == Device::DirectInput)
            return pollDirectInputRawChange_(deadzone, diCaptureGroup);

        return std::nullopt;
    }

    std::optional<RawInputEvent> RawInputPoller::pollKeyboardRawChange_()
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

    std::optional<RawInputEvent> RawInputPoller::pollXInputRawChange_(float deadzone)
    {
        XINPUT_STATE st{};
        if (GetJoypadXInputState(DX_INPUT_KEY_PAD1, &st) != 0) return std::nullopt;

        for (uint8_t bi = 0; bi < 16; ++bi)
        {
            if (st.Buttons[bi])
            {
                return RawInputEvent{ RawDevice::XInput, RawKind::Button, bi, false, 1.0f };
            }
        }
        const float thr = deadzone;
        if (st.LeftTrigger / 255.0f >= thr) return RawInputEvent{ RawDevice::XInput, RawKind::Trigger, 0, false, st.LeftTrigger / 255.0f };
        if (st.RightTrigger / 255.0f >= thr) return RawInputEvent{ RawDevice::XInput, RawKind::Trigger, 1, false, st.RightTrigger / 255.0f };
        const int athr = static_cast<int>(thr * 32767.0f + 0.5f);
        struct A { int v; uint8_t idx; };
        for (A a : { A{ st.ThumbLX,0 }, A{ st.ThumbLY,1 }, A{ st.ThumbRX,2 }, A{ st.ThumbRY,3 } })
        {
            if (a.v >= athr) return RawInputEvent{ RawDevice::XInput, RawKind::Axis, a.idx, false, static_cast<float>(a.v) / 32767.0f };
            if (a.v <= -athr) return RawInputEvent{ RawDevice::XInput, RawKind::Axis, a.idx, true,  static_cast<float>(-a.v) / 32767.0f };
        }
        return std::nullopt;
    }

    std::optional<RawInputEvent> RawInputPoller::pollDirectInputRawChange_(float deadzone, AxisGroup diCaptureGroup)
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
                        const uint8_t code = (axis == 0u) ? codeX : codeY;
                        return RawInputEvent{ RawDevice::DirectInput, RawKind::Axis, code, neg, val };
                    }
                }
                else { ctx.holding = false; }
                return std::nullopt;
            };

        auto tryPair = [&](long vx, long vy, Ctx& ctx, uint8_t codeX, uint8_t codeY)
            -> std::optional<RawInputEvent>
            {
                return capturePair(vx, vy, ctx, codeX, codeY);
            };

        if (diCaptureGroup != AxisGroup::Right)
        {
            if (auto ev = tryPair(st.X, st.Y, xy, 0, 1)) return ev;
            if (auto ev = tryPair(st.Rx, st.Ry, rxry, 3, 4)) return ev;
            if (auto ev = tryPair(st.Z, st.Rz, zrz, 2, 5)) return ev;
            if (auto ev = tryPair(st.Slider[0], st.Slider[1], s01, 6, 7)) return ev;
        }

        if (diCaptureGroup != AxisGroup::Left)
        {
            if (auto ev = tryPair(st.Rx, st.Ry, rxry, 3, 4)) return ev;
            if (auto ev = tryPair(st.Z, st.Rz, zrz, 2, 5)) return ev;
            if (auto ev = tryPair(st.X, st.Y, xy, 0, 1)) return ev;
            if (auto ev = tryPair(st.Slider[0], st.Slider[1], s01, 6, 7)) return ev;
        }

        const unsigned int pov = st.POV[0];
        if (pov != 0xFFFFFFFFu)
        {
            const bool up = (pov == 0u || pov == 4500u || pov == 31500u);
            const bool right = (pov == 9000u || pov == 4500u || pov == 13500u);
            const bool down = (pov == 18000u || pov == 13500u || pov == 22500u);
            const bool left = (pov == 27000u || pov == 22500u || pov == 31500u);
            if (up)    return RawInputEvent{ RawDevice::DirectInput, RawKind::POV, 0, false, 1.0f };
            if (right) return RawInputEvent{ RawDevice::DirectInput, RawKind::POV, 1, false, 1.0f };
            if (down)  return RawInputEvent{ RawDevice::DirectInput, RawKind::POV, 2, false, 1.0f };
            if (left)  return RawInputEvent{ RawDevice::DirectInput, RawKind::POV, 3, false, 1.0f };
        }

        for (uint8_t bi = 0; bi < 32; ++bi) if (st.Buttons[bi])
            return RawInputEvent{ RawDevice::DirectInput, RawKind::Button, bi, false, 1.0f };

        return std::nullopt;
    }
}