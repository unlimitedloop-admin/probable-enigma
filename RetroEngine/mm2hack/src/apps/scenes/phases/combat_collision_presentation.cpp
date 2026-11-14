#include "pch.h"

#include "combat_collision_presentation.h"

#include <vector>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/scenes/IStageAssetProvider.h"
#include "apps/systems/audio/AudioManager.h"
#include "apps/systems/combat/IDamageable.h"
#include "apps/systems/physics/CollisionSystem.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "apps/world/entity/common/SpawnSmallExplosionEffectCommand.h"
#include "apps/world/entity/effects/ProjectileEntity.h"
#include "apps/world/entity/effects/SmallExplosionEffectEntity.h"
#include "apps/world/entity/EntityManager.h"
#include "apps/world/entity/IEntity.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::scenes::phases
{
    namespace
    {
        /// HP before collision, used to identify targets that took damage and survived.
        struct DamageableHpSnapshot final
        {
            const systems::combat::IDamageable& damageable;
            int hp_before;
        };

        /// Spawns one destruction effect per damageable defeated during the collision pass.
        void spawn_destruction_effects(
            StageRuntimeContext& ctx, systems::audio::AudioManager& audio,
            const std::vector<systems::physics::ICollider*>& colliders)
        {
            const auto sprite_id = ctx.asset_provider->SmallExplosionEffectSprite();
            if (sprite_id == static_cast<rendering::sprite::SpriteManager::Id>(-1))
            {
                return;
            }

            for (auto* collider : colliders)
            {
                // `colliders` was snapshotted alive just before ResolveEntities() ran
                // above, so a now-dead entry died on this exact pass -- fire its
                // destruction effect exactly once, right here.
                if (collider == nullptr || collider->OwnerEntity().IsAlive())
                {
                    continue;
                }

                // Only entities with an HP pool (combat::IDamageable) get a destruction
                // effect -- e.g. a projectile despawning on its own isn't "destroyed".
                if (dynamic_cast<systems::combat::IDamageable*>(collider) == nullptr)
                {
                    continue;
                }

                const auto center = collider->Bounds().center();
                ctx.entity_mgr->Spawn<world::entity::effects::SmallExplosionEffectEntity>(
                    world::entity::common::SpawnSmallExplosionEffectCommand{
                        .spawnPos = foundation::math::Vec2{ center.x, center.y },
                        .spriteId = sprite_id,
                        .baseTexture = 0,
                    });
                audio.PlaySe(L"enemy_small_explosion");
            }
        }

        /// Records HP for each damageable collider before collision resolution.
        std::vector<DamageableHpSnapshot> capture_damageable_hp(
            const std::vector<systems::physics::ICollider*>& colliders)
        {
            std::vector<DamageableHpSnapshot> snapshot;
            snapshot.reserve(colliders.size());
            for (auto* collider : colliders)
            {
                if (collider == nullptr)
                {
                    continue;
                }

                if (auto* damageable = dynamic_cast<systems::combat::IDamageable*>(collider))
                {
                    snapshot.push_back({ *damageable, damageable->CurrentHP() });
                }
            }
            return snapshot;
        }

        /// Plays each hit sound at most once for surviving targets that lost HP.
        void play_hit_sounds(systems::audio::AudioManager& audio, const std::vector<DamageableHpSnapshot>& before)
        {
            // A kill is destruction's job (spawn_destruction_effects);
            // this is only for a hit that didn't finish the target off. Split by
            // whether the target was the player -- "be_damaged" is the player's
            // own hit-reaction SE, distinct from "hit_attack" (an enemy taking a
            // non-lethal hit from the player's own weapon).
            bool other_target_hit = false;
            bool player_hit = false;
            for (const auto& snap : before)
            {
                if (snap.damageable.IsDead() || snap.damageable.CurrentHP() >= snap.hp_before)
                {
                    continue;
                }
                if (dynamic_cast<const world::entity::avatar::PlayerEntity*>(&snap.damageable) != nullptr)
                {
                    player_hit = true;
                }
                else
                {
                    other_target_hit = true;
                }
            }

            // One shot per frame regardless of how many things got hit -- a single
            // SE channel pair, no point stacking retriggers.
            if (other_target_hit)
            {
                audio.PlaySe(L"hit_attack");
            }
            if (player_hit)
            {
                audio.PlaySe(L"be_damaged");
            }
        }

        /// Drains deflection flags and plays one sound if any projectile bounced.
        void play_deflect_sound(
            systems::audio::AudioManager& audio,
            const std::vector<systems::physics::ICollider*>& colliders)
        {
            bool any_deflected = false;
            for (auto* collider : colliders)
            {
                auto* projectile = dynamic_cast<world::entity::effects::ProjectileEntity*>(collider);
                if (projectile != nullptr && projectile->ConsumeDeflected())
                {
                    any_deflected = true;
                }
            }

            if (any_deflected)
            {
                audio.PlaySe(L"defend_shot");
            }
        }
    }

    void resolve_combat_collisions(StageRuntimeContext& ctx, systems::audio::AudioManager& audio)
    {
        // Entity-vs-entity hit detection (projectiles vs enemies/traps, player vs
        // items/traps, etc.). Runs after positions are finalized for this tick.
        std::vector<systems::physics::ICollider*> colliders;
        ctx.entity_mgr->CollectColliders(colliders);
        const auto damageable_hp_before = capture_damageable_hp(colliders);
        ctx.collision.ResolveEntities(colliders);
        spawn_destruction_effects(ctx, audio, colliders);
        play_hit_sounds(audio, damageable_hp_before);
        play_deflect_sound(audio, colliders);
    }
}
