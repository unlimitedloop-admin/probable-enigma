//==============================================================================
//
//  Project: mm2hack
//  SpawnSlidingDustEffectCommand.h
//
//  Command data used to spawn the dust shown when sliding begins.
//
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"

namespace mm2hack::apps::world::entity::common
{
    struct SpawnSlidingDustEffectCommand final
    {
        foundation::math::Vec2 spawnPos{};
        rendering::sprite::SpriteManager::Id spriteId{};
        int baseTexture{ 0 };
    };
}
