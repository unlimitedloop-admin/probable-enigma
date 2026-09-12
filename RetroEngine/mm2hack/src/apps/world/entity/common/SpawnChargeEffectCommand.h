//==============================================================================
//
//  Project: mm2hack
//  SpawnChargeEffectCommand.h
//
//  Parameters for spawning one charge-effect particle.
//
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"

namespace mm2hack::apps::world::entity::common
{
    struct SpawnChargeEffectCommand final
    {
        foundation::math::Vec2 spawnPos{};
        rendering::sprite::SpriteManager::Id spriteId{};
        int baseTexture{ 0 };
    };
}
