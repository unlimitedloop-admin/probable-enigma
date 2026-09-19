//==============================================================================
// 
//  Project: mm2hack
//  IStageAssetProvider.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "apps/rendering/bg/BGTileManager.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"

namespace mm2hack::apps::scenes
{
    using SpriteManagerId = rendering::sprite::SpriteManager::Id;
    using BGTileManagerId = rendering::bg::BGTileManager::Id;

    // Interface for providing stage asset IDs
    struct IStageAssetProvider
    {
        virtual ~IStageAssetProvider() = default;

        virtual SpriteManagerId PlayerSprite() const noexcept = 0;
        virtual SpriteManagerId PlayerChargeLevel1Sprite() const noexcept = 0;
        virtual SpriteManagerId PlayerChargeLevel2Sprite() const noexcept = 0;
        virtual SpriteManagerId PlayerAttackSprite() const noexcept = 0;
        virtual SpriteManagerId EffectsSprite() const noexcept = 0;
        virtual SpriteManagerId SlidingDustEffectSprite() const noexcept = 0;
        virtual SpriteManagerId ChargeEffectSprite() const noexcept = 0;
        virtual SpriteManagerId SmallExplosionEffectSprite() const noexcept = 0;
        // Shared across every shooting enemy kind (see AnimationTransition::
        // projectile_spawns) -- one sprite, not per-EnemyKind like TryEnemySprite.
        virtual SpriteManagerId EnemyProjectileSprite() const noexcept = 0;

        virtual bool TryEnemySprite(world::entity::enemy::EnemyKind kind, SpriteManagerId& out) const noexcept = 0;
        // Same lookup, but for a specific palette preset (see StageSpriteBank::
        // enemy_palette_variants). Index 0 is always the sheet's original colors.
        virtual bool TryEnemySprite(
            world::entity::enemy::EnemyKind kind, int palette_preset_index, SpriteManagerId& out) const noexcept = 0;

        // The stage's own BG tileset -- lets world objects (breakable blocks, etc.)
        // render using the same tile art as the background instead of a sprite sheet.
        virtual BGTileManagerId BgTilesetId() const noexcept = 0;
    };
}
