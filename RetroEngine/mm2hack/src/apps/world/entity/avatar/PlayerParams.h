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
    // Configuration for player probes offsets (All relative distances from the center)
    struct PlayerProbes
    {
        //double frontLineOffsetXR    { 8.0 };        // X offset for front line probe (right facing)
        //double frontLineOffsetXL    { -8.0 };       // X offset for front line probe (left facing)
        double frontLineOffsetX     { 8.0 };        // X offset for front line probe
        double rearLineOffsetX      { -8.0 };       // X offset for rear line probe
        double verticalTopOffsetY   { -14.0 };      // Y offset for top line probe
        double verticalMidOffsetY   { -7.0 };       // Y offset for middle line probe
        double verticalBtmOffsetY   { 12.0 };       // Y offset for bottom line probe

        double groundLineOffsetY    { 9.0 };        // Y offset for ground line probe
        double overheadLineOffsetY  { -14.0 };      // Y offset for overhead line probe
        double horizonFrontOffsetX  { 7.0 };        // X offset for horizon front probe
        double horizonMidOffsetX    { 0.0 };        // X offset for horizon middle probe
        double horizonBehindOffsetX { -7.0 };       // X offset for horizon behind probe
    };

    // Controlling avatar parameters for physics and movement
    struct PlayerTuning
    {
        double gravity              { 0x00.40p0 };  // Falling Newton velocity
        double jumpImpulse          { -0x04.DFp0 }; // Resistance value during takeoff
        double terminalVelocity     { 0x13.00p0 };  // Max falling velocity

        double fallingThreshold     { -0x02.20p0 }; // Falling threshold velocity
        double jumpCutVelocity      { -0x01.00p0 }; // Velocity when jump is cut
        double airStrafeVelocity    { 0x01.50p0 };  // Air control horizontal velocity

        uint8_t reactionFrameRun    { 6 };          // Beginning frame of running animation

        double momentumStart        { 0x00.20p0 };  // Launch run speed (Fixed)
        double steadyRun            { 0x01.60p0 };  // Normal run speed
        double haltSpeed            { 0x00.80p0 };  // Brake run speed (Fixed)

        PlayerProbes probeOffsets;                  // Player probes offsets
    };
}