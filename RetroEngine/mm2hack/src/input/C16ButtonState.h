//==============================================================================
// 
//  Project: mm2hack
//  C16ButtonState.h
// 
//  Manages the pressed and released frames of 16 buttons.
// 
//==============================================================================
#pragma once

#include <array>
#include "InputFrame.h"
#include "Jpbtn.h"

namespace mm2hack::input
{
    // A class to track the pressed and released frames of 16 buttons
    class C16ButtonState
    {
    public:
        // Update the state of a button based on its index and whether it is pressed
        void UpdateButton(size_t index, bool isPressed);
        // Get the pressed and released frames of a button based on its index
        const InputFrame& GetState(size_t index) const;
        // Get the pressed and released frames of all buttons
        const std::array<InputFrame, JPBTN_COUNT>& GetAllStates() const;
        void ResetAll();

    private:
        std::array<InputFrame, JPBTN_COUNT> _states;    // Array to hold the states of the buttons
    };
}