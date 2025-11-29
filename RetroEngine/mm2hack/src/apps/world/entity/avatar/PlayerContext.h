//==============================================================================
// 
//  Project: mm2hack
//  PlayerContext.h
// 
//  Context for player state management.
// 
//==============================================================================
#pragma once

#include "abilities/ServiceModules.h"
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/Probes.h"
#include "apps/world/entity/common/AnimeStepper.h"
#include "AvatarStatus.h"

namespace mm2hack::apps::world::entity::avatar
{
    using abilities::ILadderService;
    using common::AnimeStepper;
    using foundation::math::Vec2;
    using systems::physics::Probes;
    using systems::physics::ITerrainProbe;

    // Context passed to player state handlers
    struct PlayerContext
    {
        // Needed status for avatar state updates
        Vec2& pos;                              // x/y position
        Vec2& vel;                              // x/y velocity
        bool& onGround;                         // Is the avatar on the ground?
        AvatarDirection& facingLR;              // avatar facing direction (-1: left, +1: right)
        int& texture;                           // tile index for rendering

        AnimeStepper& animeStepper;             // Animation counter (local)
        Probes& probes;                         // All probes
        
        const ITerrainProbe* terrain;           // Look up interface for terrain probing
        ILadderService* ladder{ nullptr };      // Ladder service module
    };
}