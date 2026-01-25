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
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/scrolling/atomic/IScrollRuleProvider.h"
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
    using rendering::sprite::SpriteManager;
    using systems::physics::ILadderService;
    using systems::physics::ITerrainProbe;
    using systems::physics::Probes;
    using systems::scrolling::atomic::FixedScrollRequest;
    using systems::scrolling::atomic::IScrollRuleProvider;

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
        rendering::sprite::SpriteManager::Id id;            // sprite id for rendering

        Vec2& pos;                                          // x/y position
        Vec2& vel;                                          // x/y velocity
        bool& onGround;                                     // Is the avatar on the ground?
        bool justLanded{ false };                           // Did the avatar just land this frame?
        bool isHitCeiling{ false };                         // Is the avatar hitting the ceiling?
        const bool prevOnGround{ false };                   // Was the avatar on the ground previous frame?
        AvatarDirection& facingLR;                          // avatar facing direction (-1: left, +1: right)
        int& texture;                                       // tile index for rendering
        int textureAdd{ 0 };                                // Texture index addition for rendering

        AnimeStepper& animeStepper;                         // Animation counter (local)
        Probes& probes;                                     // All probes
        Probes& prelimProbes;                               // Preliminary probes before movement
        RectF bounds;                                       // Get bounding box for convenience
        Vec2 pageOriginPx;                                  // Current page origin in world px

        const ITerrainProbe* terrain;                       // Look up interface for terrain probing
        ILadderService* ladder{ nullptr };                  // Laddering service module

        WorldBounds vBounds;                                // VRAM area boundaries
        const IScrollRuleProvider* scrollRules{ nullptr };  // Scroll rule provider
        std::size_t scrollPageIndex{ 0 };                   // Current page index for scrolling

        FixedScrollRequest pendingFixedScroll{};            // Pending fixed scroll request
    };

    // Simplified context for animation abilities
    struct AnimeContext
    {
        AnimeStepper& animeStepper;                         // Animation counter (local)
        AvatarDirection& facingLR;                          // avatar facing direction (-1: left, +1: right)
        int& texture;                                       // tile index for rendering
    };

    // Extended context for entity-level data struct (snapshots)
    struct ExPlayerContextForEntity
    {
        bool canSpawnProjectile{ false };                   // Whether the player can spawn projectiles now
    };
}