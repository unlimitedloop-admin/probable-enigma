#include "pch.h"

#include "KeyboardInputProvider.h"

#include "C16ButtonState.h"
#include "Jpbtn.h"

namespace mm2hack::input
{
    bool KeyboardInputProvider::Update(C16ButtonState& out_state)
    {
        if (!_keyboard->UpdateAllStateKey()) return false;

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            uint16_t keycode = _binding.GetBindingSCon(static_cast<JPBTN>(i));
            int64_t pressed = _keyboard->GetHoldKeyValue(keycode);
            out_state.UpdateButton(i, pressed > 0);
        }
        return true;
    }
}