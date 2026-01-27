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
#include "apps/systems/view/RenderContext.h"

namespace mm2hack::apps::world::entity::common
{
    // Command structure for spawning a projectile entity in the game world
    struct SpawnProjectileCommand final
    {
        foundation::math::Vec2 spawnPos{};                                  // Spawn position of the projectile
        foundation::math::Vec2 velocity{};                                  // Initial velocity of the projectile
        systems::view::Layer drawLayer{ systems::view::Layer::Effects };    // Which drawing layer to use

        rendering::sprite::SpriteManager::Id spriteId{};                    // Which sprite-set to use for drawing (same as PlayerEntity::_id etc.)
        int baseTexture{ 0 };                                               // Base texture index for the projectile
        std::int32_t animFrames{ 1 };                                       // Number of animation frames for the projectile
        double animFps{ 0.0 };                                              // Animation speed (frames per second)

        double lifeSec{ 1.0 };                                              // Lifetime of the projectile in seconds
    };
}