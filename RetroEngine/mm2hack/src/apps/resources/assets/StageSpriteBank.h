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
        SpriteManagerId small_explosion_effect{};
        SpriteManagerId miss_bubble_effect{};
        SpriteManagerId health_meter{};
        SpriteManagerId enemy_projectile{};

        std::unordered_map<world::entity::enemy::EnemyKind, SpriteManagerId> enemies{};
        // [kind][palette preset index into EnemyDefinition::palette_presets] -> recolored sprite id.
        // Index 0 is always present and is the sheet's original (unrecolored) colors.
        std::unordered_map<world::entity::enemy::EnemyKind, std::unordered_map<int, SpriteManagerId>> enemy_palette_variants{};
    };
}
