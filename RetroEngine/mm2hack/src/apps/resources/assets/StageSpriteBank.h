//==============================================================================
// 
//  Project: mm2hack
//  StageSpriteBank.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <unordered_map>

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"

namespace mm2hack::apps::resources::assets
{
    using SpriteManagerId = apps::rendering::sprite::SpriteManager::Id;

    // Stage asset collection
    struct StageSpriteBank final
    {
        SpriteManagerId player{};
        SpriteManagerId player_charge_level1{};
        SpriteManagerId player_charge_level2{};
        SpriteManagerId player_attack{};
        SpriteManagerId effects{};
        SpriteManagerId sliding_dust_effect{};
        SpriteManagerId charge_effect{};

        std::unordered_map<world::entity::enemy::EnemyKind, SpriteManagerId> enemies{};
    };
}
