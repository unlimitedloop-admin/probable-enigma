//==============================================================================
// 
//  Project: mm2hack
//  AvatarStatus.h
// 
//  Avatar status definitions.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::world::entity::avatar
{
    // Avatar basic status enumeration
    enum class AvatarStatus : std::int8_t
    {
        Disabled = -1,
        Uncontrollable = 0,
        Setback,
        Damaged,
        Standing,
        LaunchRun,
        BrakeRun,
        Running,
        Hovering,
        Landing,
        Ladder,
        // ... Add more statuses as needed ;)
    };

    // Avatar facing direction
    enum class AvatarDirection : std::int8_t
    {
        Left  = -1,
        Right = +1,
    };

    enum class AvatarAnimation : std::int8_t
    {
        ToTheRight = 0,
        StandingA,
        StandingB,
        RunningIntro,
        RunningA,
        RunningB,
        RunningC,
        Airpause,
        ToTheLeft = 40,
        // ... Add more animations as needed ;)
    };

    using STile = AvatarAnimation;
}