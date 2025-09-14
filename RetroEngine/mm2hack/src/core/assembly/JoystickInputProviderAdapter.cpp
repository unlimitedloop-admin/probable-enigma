#include "pch.h"

#include <cstdint>
#include "InputTypes.h"
#include "JoystickInputProviderAdapter.h"

namespace mm2hack::core::assembly
{
    void JoystickInputProviderAdapter::BeginTick(std::uint64_t tick) noexcept
    {
        _tick = tick;

        // The joystick has already been updated externally (in Sequence object).
        // Here, we only check if it is currently pressed. The count is updated on the "tick" side.
        for (size_t i = 0; i < _state.size(); ++i)
        {
            bool now = _jm.GetButtonState(i).is_pressed;
            auto& s = _state[i];
            bool prev = s.pressed;
            s.pressed = now;
            s.changed = (prev != now);
            s.frames = now ? (prev && s.frames > 0 ? s.frames + 1 : 1)
                : (!prev && s.frames < 0 ? s.frames - 1 : -1);
        }
    }

    InputSnapshot JoystickInputProviderAdapter::GetSnapshot() const
    {
        InputSnapshot ss;
        ss.keys = _state;
        ss.tick = _tick;
        return ss;
    }
}