//==============================================================================
// 
//  Project: mm2hack
//  SpawnProjectileCommand.h
// 
//  Command structure for spawning a projectile entity in the game world.
// 
//==============================================================================
#pragma once

#include <cstdint>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/view/RenderContext.h"

namespace mm2hack::apps::world::entity::common
{
    struct SpawnProjectileCommand final
    {
        foundation::math::Vec2 spawnPos{};
        foundation::math::Vec2 velocity{};
        systems::view::Layer drawLayer{ systems::view::Layer::Effects };

        rendering::sprite::SpriteManager::Id spriteId{};       // Which sprite-set to use for drawing (same as PlayerEntity::_id etc.)
        int baseTexture{ 0 };
        std::int32_t animFrames{ 1 };
        double animFps{ 0.0 };

        double lifeSec{ 1.0 };
    };
}