//==============================================================================
// 
//  Project: mm2hack
//  AnimeStepper.h
// 
//  Generative counter for animation frames, holding any necessary, etc...
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::world::entity::common
{
    // The tick is counted, the frame is the weight, and it repeats for the specified number of loops as an animation counter
    struct AnimeStepper
    {
        int tick{ 0 };
        int frame{ 0 };
        int loops{ 0 };

        // returns: true if the animation has completed the specified loop count
        bool step(int ticksPerFrame, int frameCount, int loopCount = 1) noexcept
        {
            if (ticksPerFrame <= 0 || frameCount <= 0 || loopCount <= 0)
            {
                return false;
            }

            if (++tick < ticksPerFrame)
            {
                return false;
            }

            tick = 0;

            // Advance frame
            ++frame;
            if (frame >= frameCount)
            {
                frame = 0;
                ++loops;
            }

            return loops >= loopCount;
        }

        // All reset in one
        void reset() noexcept
        {
            tick = 0;
            frame = 0;
            loops = 0;
        }

        // Reset only tick or frame
        void resetTick() noexcept
        {
            tick = 0;
        }

        // Reset only frame
        void resetFrame() noexcept
        {
            frame = 0;
        }
    };
}