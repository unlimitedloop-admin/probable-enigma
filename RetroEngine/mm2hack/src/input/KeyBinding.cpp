#include "pch.h"

#include "KeyBinding.h"

#include <array>
#include <cstdint>
#include "DefaultKeyArray.h"
#include "Jpbtn.h"

namespace mm2hack::input
{
    bool KeyBinding::SetBindingSCon(const std::array<uint16_t, JPBTN_COUNT>& buttonMap, bool xInput, bool hatswc, bool trgg, bool thumb)
    {
        _sCon.xinput_enabled = xInput;
        if (!_sCon.xinput_enabled)
        {
            for (size_t i = 0; i < JPBTN_COUNT; ++i)
            {
                if (buttonMap[i] > 0xFF)
                {
                    return false;
                }
                _sCon.button_map[i] = buttonMap[i];
            }
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
            return 0xFFFF; // Invalid button index
        }

        return _sCon.button_map[index];
    }

    bool KeyBinding::IsXInputEnabled() const
    {
        return _sCon.xinput_enabled;
    }

    bool KeyBinding::SetDefaultBindingSCon()
    {
        if (DxLib::GetJoypadNum())
        {
            return SetBindingSCon({}, true, true, true, true);
        }
        else
        {
            return SetBindingSCon(GetDefaultKeyArray(), false, false, false, false);
        }
    }
}