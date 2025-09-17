//==============================================================================
// 
//  Project: mm2hack
//  InputFrame.h
// 
//  The key has been pressed or released.
//
//==============================================================================
#pragma once

namespace mm2hack::input
{
    // State of a single key for the current frame
    struct InputFrame
    {
        bool is_pressed = false;

        // Update the pressed and released frames based on the current state
        void Update(bool is_pressed)
        {
            if (is_pressed)
            {
                this->is_pressed = true;
            }
            else
            {
                this->is_pressed = false;
            }
        }

        // Reset the frame counters
        void Reset()
        {
            is_pressed = false;
        }
    };
}