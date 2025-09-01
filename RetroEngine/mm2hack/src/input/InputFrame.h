//==============================================================================
// 
//  Project: mm2hack
//  InputFrame.h
// 
//  Frame tracking for input states in the game.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::input
{
    // Tracks the state of an input frame, including the number of frames
    struct InputFrame
    {
        int64_t pressed_frame = 0;
        int64_t released_frame = 0;

        // Update the pressed and released frames based on the current state
        void Update(bool is_pressed)
        {
            if (is_pressed)
            {
                ++pressed_frame;
                released_frame = 0;
            }
            else
            {
                ++released_frame;
                pressed_frame = 0;
            }
        }

        // Reset the frame counters
        void Reset()
        {
            pressed_frame = 0;
            released_frame = 0;
        }
    };
}