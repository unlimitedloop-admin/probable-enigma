#include "KeyboardsIn.h"

#include <array>
#include <cstdint>
#include <DxLib.h>

namespace mm2hack::input
{
    bool KeyboardsIn::UpdateAllStateKey()
    {
        std::array<int, KEY_NUM> key_frame_array{};
        if (-1 == DxLib::GetHitKeyStateAllEx(key_frame_array.data())) { return false; }

        for (int i = 0; i < KEY_NUM; i++)
        {
            _diKeyPressed[i] = key_frame_array[i];
        }
        return true;
    }

    int64_t KeyboardsIn::GetHoldKeyValue(size_t keynumber)
    {
        return (KEY_NUM <= keynumber) ? -1 : _diKeyPressed[keynumber];
    }
}