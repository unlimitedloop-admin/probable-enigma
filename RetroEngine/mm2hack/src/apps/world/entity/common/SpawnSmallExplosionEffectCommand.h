//==============================================================================
//
//  Project: mm2hack
//  SpawnSmallExplosionEffectCommand.h
//
//  Command data used to spawn a small explosion/destruction effect entity.
//
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"

namespace mm2hack::apps::world::entity::common
{
    // Command structure for spawning a small explosion effect in the game world
    struct SpawnSmallExplosionEffectCommand final
    {
        foundation::math::Vec2 spawnPos{};                       // Center position of the explosion
        rendering::sprite::SpriteManager::Id spriteId{};         // Explosion sprite-set ID
        int baseTexture{ 0 };                                    // First tile index (tile 0 of the sheet) for this sprite-set
    };
}
