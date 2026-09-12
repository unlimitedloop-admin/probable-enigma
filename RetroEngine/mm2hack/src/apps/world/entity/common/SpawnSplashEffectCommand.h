//==============================================================================
//
//  Project: mm2hack
//  SpawnSplashEffectCommand.h
//
//  Command data used to spawn a splash effect entity.
//
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"

namespace mm2hack::apps::world::entity::common
{
    // Command structure for spawning a splash effect in the game world
    struct SpawnSplashEffectCommand final
    {
        foundation::math::Vec2 spawnPos{};                       // Center position on the water surface
        rendering::sprite::SpriteManager::Id spriteId{};         // Splash sprite-set ID
        int baseTexture{ 0 };                                    // First animation frame for the captured direction
    };
}
