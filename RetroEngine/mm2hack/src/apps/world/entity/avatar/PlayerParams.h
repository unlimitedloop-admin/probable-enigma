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
#include "apps/foundation/math/CoordinateTypes.h"
#include "AvatarStatus.h"

namespace mm2hack::apps::world::entity::avatar
{
    // Configuration for player probes offsets (All relative distances from the center)
    struct PlayerProbes
    {
        double frontLineOffsetX     { 8.0 };        // X offset for front line probe
        double rearLineOffsetX      { -8.0 };       // X offset for rear line probe
        double verticalTopOffsetY   { -14.0 };      // Y offset for top line probe
        double verticalMidOffsetY   { -7.0 };       // Y offset for middle line probe
        double verticalBtmOffsetY   { 8.0 };        // Y offset for bottom line probe
        double verticalBtmOffsetY2  { 9.0 };        // Y offset for second bottom line probe (Probes in hovering)

        double groundLineOffsetY    { 9.0 };        // Y offset for ground line probe
        double overheadLineOffsetY  { -15.0 };      // Y offset for overhead line probe
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
        double climbSpeed           { 0x00.C0p0 };  // Climbing speed on ladder

        PlayerProbes probeOffsets;                  // Player probes offsets
    };

    // Structure to represent ground movement intent, Pre-declare this data type for use with ApplyGroundMove
    struct GroundMoveIntent
    {
        int dirSign{ 0 };                           // -1 left, +1 right, 0 none
        double speed{ 0.0 };                        // absolute horizontal speed (>=0)
        bool active{ false };                       // should apply this frame?
    };

    // Structure to represent air movement intent, Pre-declare this data type for use with ApplyAirControl
    struct AirMoveIntent
    {
        int dirSign{ 0 };                           // -1 left, +1 right, 0 none
        double speed{ 0.0 };                        // absolute horizontal speed (>=0)
        bool active{ false };                       // should apply this frame?
    };

    // Intro drop phases
    enum class IntroPhase
    {
        Falling,
        Landing,
        Done
    };

    // Intro drop state
    struct IntroDropState
    {
        bool active{ false };                       // Is on stage sequence active
        IntroPhase phase{ IntroPhase::Falling };    // Current phase of the intro
        double timer{ 0.0 };                        // Timer for phase transitions
        foundation::math::Vec2 offsetPos{};         // Starting position off stage
        foundation::math::Vec2 destPos{};           // Target position to reach on stage
        double dropDuration{ 0.5 };                 // Duration of the drop phase in seconds
    };

    struct IntroFrame
    {
        STile tile;                                 // Texture index for the frame
        double duration;                            // Duration to display the frame
    };
}