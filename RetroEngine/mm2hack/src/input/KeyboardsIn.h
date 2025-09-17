//==============================================================================
// 
//  Project: mm2hack
//  KeyboardsIn.h
// 
//  Manages the all state of keyboard inputs.
// 
//==============================================================================
#pragma once

#include <array>
#include <cstdint>

namespace mm2hack::input
{
    // Manages the all state of keyboard inputs
    class KeyboardsIn
    {
    public:
        KeyboardsIn() : _diKeyPressed{} {}
        ~KeyboardsIn() {}
        KeyboardsIn(const KeyboardsIn&) = delete;
        KeyboardsIn& operator=(const KeyboardsIn&) = delete;
        KeyboardsIn(KeyboardsIn&&) = delete;
        KeyboardsIn& operator=(KeyboardsIn&&) = delete;
        // KeyboardsIn is a singleton, so we delete the copy and move constructors and assignment operators.

        // Update the state of all keys and return true if successful
        bool UpdateAllStateKey();
        // Get the hold state of a specific key by its number
        int64_t GetHoldKeyValue(size_t keynumber);

    private:
        static const size_t KEY_NUM = 256;
        std::array<int64_t, KEY_NUM> _diKeyPressed{};
    };
}