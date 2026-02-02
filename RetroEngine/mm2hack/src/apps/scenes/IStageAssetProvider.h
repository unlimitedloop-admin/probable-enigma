//==============================================================================
// 
//  Project: mm2hack
//  IStageAssetProvider.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"

namespace mm2hack::apps::scenes
{
    using SpriteManagerId = rendering::sprite::SpriteManager::Id;

    // Interface for providing stage asset IDs
    struct IStageAssetProvider
    {
        virtual ~IStageAssetProvider() = default;

        virtual SpriteManagerId PlayerSprite() const noexcept = 0;
        virtual SpriteManagerId PlayerAttackSprite() const noexcept = 0;
        virtual SpriteManagerId EffectsSprite() const noexcept = 0;

        virtual bool TryEnemySprite(world::entity::enemy::EnemyKind kind, SpriteManagerId& out) const noexcept = 0;
    };
}