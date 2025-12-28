//==============================================================================
// 
//  Project: mm2hack
//  PlayerContext.h
// 
//  Context for player state management.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/world/entity/common/AnimeStepper.h"
#include "AvatarStatus.h"

namespace mm2hack::apps::systems::physics
{
    struct Probes;
    class ITerrainProbe;
}

namespace mm2hack::apps::world::entity::avatar
{
    using common::AnimeStepper;
    using foundation::math::RectF;
    using foundation::math::Vec2;
    using systems::physics::ILadderService;
    using systems::physics::ITerrainProbe;
    using systems::physics::Probes;
    using systems::scrolling::atomic::PageScroll;

    // World boundary representation
    struct WorldBounds
    {
        double leftX{};
        double rightX{};   // inclusive
        double topY{};
        double bottomY{};
    };

    // Context passed to player state handlers
    struct PlayerContext
    {
        // Needed status for avatar state updates
        Vec2& pos;                              // x/y position
        Vec2& vel;                              // x/y velocity
        bool& onGround;                         // Is the avatar on the ground?
        bool justLanded{ false };               // Did the avatar just land this frame?
        bool isHitCeiling{ false };             // Is the avatar hitting the ceiling?
        const bool prevOnGround{ false };       // Was the avatar on the ground previous frame?
        AvatarDirection& facingLR;              // avatar facing direction (-1: left, +1: right)
        int& texture;                           // tile index for rendering

        AnimeStepper& animeStepper;             // Animation counter (local)
        Probes& probes;                         // All probes
        Probes& prelimProbes;                   // Preliminary probes before movement
        RectF bounds;                           // Get bounding box for convenience

        const ITerrainProbe* terrain;           // Look up interface for terrain probing
        ILadderService* ladder{ nullptr };      // Laddering service module

        WorldBounds vBounds;                    // VRAM area boundaries
        PageScroll::Dir pendingFixedScroll{
            PageScroll::Dir::None };            // Pending fixed scroll request
    };
}