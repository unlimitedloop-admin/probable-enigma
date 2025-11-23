//==============================================================================
// 
//  Project: mm2hack
//  PlayerParams.h
// 
//  Player parameters definitions.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::world::entity::avatar
{
    // Input snapshot structure
    struct InputSnapshot
    {
        bool left { false };
        bool right{ false };
        bool up   { false };
        bool down { false };
        bool jump { false };
        bool fire { false };
    };

    struct PlayerTuning
    {
        double gravity          { 0x00.40p0 };  // Falling Newton velocity
        double jumpImpulse      { -0x04.DFp0 }; // Resistance value during takeoff
        double terminalVelocity { 0x13.00p0 };  // Max falling velocity

        double fallingThreshold { -0x02.20p0 }; // Falling threshold velocity
        double jumpCutVelocity  { -0x01.00p0 }; // Velocity when jump is cut

        uint8_t reactionFrameRun{ 6 };          // Invincibility frames after running into an 

        double momentumStart    { 0x00.20p0 };  // Launch run speed (Fixed)
        double steadyRun        { 0x01.60p0 };  // Normal run speed
        double haltSpeed        { 0x00.80p0 };  // Brake run speed (Fixed)
    };
}