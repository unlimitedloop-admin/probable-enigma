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
    struct AnimeStepper
    {
        int tick{ 0 };
        int cycle{ 0 };

        void step(int limitTick, int limitCycle)
        {
            if (++tick >= limitTick)
            {
                tick = 0;
                cycle = (cycle + 1) % limitCycle;
            }
        }

        void reset()
        {
            tick = 0;
            cycle = 0;
        }
    };
}