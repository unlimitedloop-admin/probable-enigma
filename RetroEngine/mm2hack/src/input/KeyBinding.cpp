#include "pch.h"

#include "KeyBinding.h"

#include <cstdint>
#include "DefaultKeyArray.h"
#include "Jpbtn.h"
#include "KeyToken.h"

namespace mm2hack::input
{
    bool KeyBinding::SetBinding(JPBTN button, uint16_t token)
    {
        const size_t idx = static_cast<size_t>(button);
        if (idx >= JPBTN_COUNT) return false;
        _sCon.button_map[idx] = token;
        return true;
    }

    bool KeyBinding::UnsetBinding(JPBTN button)
    {
        const size_t idx = static_cast<size_t>(button);
        if (idx >= JPBTN_COUNT) return false;
        _sCon.button_map[idx] = kTokenUnbound;
        return true;
    }

    bool KeyBinding::SetBindingSCon(const std::array<uint16_t, JPBTN_COUNT>& buttonMap, bool xInput, bool hatswc, bool trgg, bool thumb)
    {
        _sCon.xinput_enabled = xInput;
        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            _sCon.button_map[i] = buttonMap[i];
        }
        _sCon.hatswitch_enabled = hatswc;
        _sCon.trigger_enabled = trgg;
        _sCon.thumb_enabled = thumb;

        return true;
    }

    uint16_t KeyBinding::GetBindingSCon(JPBTN button) const
    {
        const size_t index = static_cast<size_t>(button);
        if (index >= JPBTN_COUNT)
        {
            return kTokenUnbound;   // Invalid button index
        }

        return _sCon.button_map[index];
    }

    const std::array<uint16_t, JPBTN_COUNT>& KeyBinding::GetAllBindingsSCon() const noexcept
    {
        return _sCon.button_map;
    }

    bool KeyBinding::SetDefaultBindingSCon(Device type)
    {
        if (type == Device::XInput)
        {
            return SetBindingSCon(GetDefaultXInputArray(), true, true, true, true);
        }
        else if (type == Device::DirectInput)
        {
            return SetBindingSCon(GetDefaultDirectInputArray(), false, true, true, true);
        }
        else // Keyboard or others
        {
            return SetBindingSCon(GetDefaultKeyArray(), false, false, false, false);
        }
    }
}