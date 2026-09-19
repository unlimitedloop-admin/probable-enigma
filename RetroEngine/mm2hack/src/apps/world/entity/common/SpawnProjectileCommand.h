//==============================================================================
// 
//  Project: mm2hack
//  SpawnProjectileCommand.h
// 
//  Trait structure for spawning projectile entities.
// 
//==============================================================================
#pragma once

#include <cstdint>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/WeaponId.h"
#include "apps/systems/view/RenderContext.h"

namespace mm2hack::apps::world::entity::common
{
    enum class ProjectileVisual : std::uint8_t
    {
        Normal,
        ChargeLevel1,
        ChargeLevel2
    };

    // Command structure for spawning a projectile entity in the game world
    struct SpawnProjectileCommand final
    {
        foundation::math::Vec2 spawnPos{};                                  // Spawn position of the projectile
        foundation::math::Vec2 velocity{};                                  // Initial velocity of the projectile
        systems::view::Layer drawLayer{ systems::view::Layer::Effects };    // Which drawing layer to use

        rendering::sprite::SpriteManager::Id spriteId{};                    // Which sprite-set to use for drawing (same as PlayerEntity::_id etc.)
        int baseTexture{ 0 };                                               // Base texture index for the projectile
        ProjectileVisual visual{ ProjectileVisual::Normal };                // Single-tile or composite drawing pattern
        std::int32_t animFrames{ 1 };                                       // Number of animation frames for the projectile
        double animFps{ 0.0 };                                              // Animation speed (frames per second)

        double lifeSec{ 1.0 };                                              // Lifetime of the projectile in seconds

        int power{ 1 };                                                     // Attack power (damage) carried on contact
        foundation::math::Vec2 hitHalfSize{ 2.0, 2.0 };                     // Half-size of the attack hit judgement box (independent of the sprite's draw size)

        // Which collidable layer this shot belongs to, and which weapon it's
        // reported as for resistance lookups. Default to the player's own Rock
        // Buster so every existing spawn call keeps working unchanged; an enemy
        // shot passes CollisionLayer::ProjectileEnemy + WeaponId::EnemyShot.
        systems::physics::CollisionLayer collisionLayer{ systems::physics::CollisionLayer::ProjectilePlayer };
        systems::physics::WeaponId weapon{ systems::physics::WeaponId::Buster };
    };
}
