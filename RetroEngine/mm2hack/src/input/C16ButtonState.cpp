#include "pch.h"

#include "C16ButtonState.h"

#include <array>
#include "InputFrame.h"
#include "Jpbtn.h"

namespace mm2hack::input
{
    void C16ButtonState::UpdateButton(size_t index, bool isPressed)
    {
        if (index < JPBTN_COUNT)
        {
            _states[index].Update(isPressed);
        }
    }

    const InputFrame& C16ButtonState::GetState(size_t index) const
    {
        return _states.at(index);
    }

    const std::array<InputFrame, JPBTN_COUNT>& C16ButtonState::GetAllStates() const
    {
        return _states;
    }

    void C16ButtonState::ResetAll()
    {
        for (auto& state : _states)
        {
            state.Reset();
        }
    }
}