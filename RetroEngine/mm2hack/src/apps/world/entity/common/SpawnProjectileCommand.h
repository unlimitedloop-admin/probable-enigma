//==============================================================================
// 
//  Project: mm2hack
//  SpawnProjectileCommand.h
// 
//  ** Descriptions **
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
        foundation::math::Vec2 spawn_pos{};
        foundation::math::Vec2 velocity{};
        systems::view::Layer draw_layer{ systems::view::Layer::Effects };

        rendering::sprite::SpriteManager::Id sprite_id{};       // Which sprite-set to use for drawing (same as PlayerEntity::_id etc.)
        int base_texture{ 0 };
        std::int32_t anim_frames{ 1 };
        double anim_fps{ 0.0 };

        double life_sec{ 1.0 };
    };
}