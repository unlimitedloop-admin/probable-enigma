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
        rendering::sprite::SpriteManager::Id weaponId;      // weapon sprite id for rendering

        Vec2& pos;                                          // x/y position
        Vec2& vel;                                          // x/y velocity
        bool& onGround;                                     // Is the avatar on the ground?
        bool justLanded{ false };                           // Did the avatar just land this frame?
        bool isHitCeiling{ false };                         // Is the avatar hitting the ceiling?
        const bool prevOnGround{ false };                   // Was the avatar on the ground previous frame?
        AvatarDirection& facingLR;                          // avatar facing direction (-1: left, +1: right)
        int basePose{ 0 };                                 // Base pose for the avatar
        int textureAdd{ 0 };                                // Texture index addition for rendering

        AnimeStepper& animeStepper;                         // Animation counter (local)
        Probes& probes;                                     // All probes
        Probes& prelimProbes;                               // Preliminary probes before movement
        RectF bounds;                                       // Get bounding box for convenience
        Vec2 pageOriginPx;                                  // Current page origin in world px

        const ITerrainProbe* terrain;                       // Look up interface for terrain probing
        ILadderService* ladder{ nullptr };                  // Laddering service module
        bool lockClimbMove{ false };                        // Lock climbing movement (for laddering state)

        WorldBounds vBounds;                                // VRAM area boundaries
        const IScrollRuleProvider* scrollRules{ nullptr };  // Scroll rule provider
        std::size_t scrollPageIndex{ 0 };                   // Current page index for scrolling

        FixedScrollRequest pendingFixedScroll{};            // Pending fixed scroll request

        // Jump button edge for this state Update() call. Equivalent to StateProvider::JustPressed(JPBTN::A)
        // in every normal case; while underwater physics-skip is active, PlayerEntity latches a press that
        // landed on a skipped tick and reports it here on the next tick that actually runs, so a jump input
        // is never silently dropped by the skip gate. States should read this instead of calling
        // in->JustPressed(JPBTN::A) directly.
        bool jumpEdge{ false };
    };

    // Simplified context for animation abilities
    struct AnimeContext
    {
        AnimeStepper& animeStepper;                         // Animation counter (local)
        AvatarDirection& facingLR;                          // avatar facing direction (-1: left, +1: right)
        int& basePose;                                     // Base pose for rendering
        int& textureAdd;                                    // Texture index addition for rendering
    };

    // Extended context for entity-level data struct (snapshots)
    struct ExPlayerContextForEntity
    {
        bool canSpawnProjectile{ true };                    // Whether the player can spawn projectiles now
    };
}