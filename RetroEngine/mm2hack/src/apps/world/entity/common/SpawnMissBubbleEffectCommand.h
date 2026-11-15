//==============================================================================
//
//  Project: mm2hack
//  SpawnMissBubbleEffectCommand.h
//
//  Command data used to spawn one bubble of the player's miss (death) effect.
//
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"

namespace mm2hack::apps::world::entity::common
{
    // Command structure for spawning one scattering miss bubble in the game world
    struct SpawnMissBubbleEffectCommand final
    {
        foundation::math::Vec2 spawnPos{};                       // Center position the bubble starts from (the player's position)
        foundation::math::Vec2 velocity{};                       // Constant drift, in px per tick
        rendering::sprite::SpriteManager::Id spriteId{};         // Bubble sprite-set ID
    };
}
