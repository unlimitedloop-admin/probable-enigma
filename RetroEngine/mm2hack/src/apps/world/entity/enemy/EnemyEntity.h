//==============================================================================
//
//  Project: mm2hack
//  EnemyEntity.h
//
//  Generic, data-driven enemy. One class covers every EnemyKind: sprite id,
//  which palette preset (color), toughness (resistance), a movement speed
//  multiplier, and the animation state graph itself are all supplied per
//  spawn rather than hardcoded per kind. Always faces the player and hops
//  toward them per its animation state graph's own movement/timing (e.g.
//  Met's proximity-triggered rise-shoot-jump); per-kind movement AI beyond
//  that (fly patterns, ranged retreat, ...) is future work, layered on top
//  of the animation::AnimationStatePlayer this entity already drives.
//
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <cstdint>
#include <utility>
#include <vector>

#include "animation/AnimationStatePlayer.h"
#include "animation/AnimationTypes.h"
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/combat/DamageTable.h"
#include "apps/systems/combat/HealthComponent.h"
#include "apps/systems/combat/IDamageable.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/IAttackInfo.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/systems/physics/IDeflector.h"
#include "apps/systems/physics/SimpleGravityBody.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"
#include "apps/world/entity/IEntity.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::enemy
{
    struct EnemyEntityState final
    {
        EntityKinematicState kinematic{};
        EnemyKind kind{ EnemyKind::Met };
        // Index into this kind's EnemyDefinition::palette_presets (see
        // EnemyDefinitionCatalog) -- not a live SpriteManager::Id, so it can be
        // re-resolved against whatever got (re)loaded on restore.
        std::int32_t palette_preset_index{ 0 };
        std::int32_t facing_texture_offset_left{};     // Added to the current tile while facing left
        // 0 = invincible (immune to every weapon, HP never moves); N = dies
        // after N hits' worth of accumulated normal-shot power.
        std::int32_t toughness{ 1 };
        std::int32_t hp{ 1 };                          // Current HP (meaningless/always full when toughness == 0)
        foundation::math::Vec2 half_size{ 8.0, 8.0 };
        // Independent of half_size, the same way ProjectileEntity/PlayerEntity
        // keep a hit box separate from their render footprint -- see
        // EnemyEntity::_hitbox_half.
        foundation::math::Vec2 hitbox_half_size{ 8.0, 8.0 };
        // Shifts the hit box's center down from pos (Y only, same idea as
        // kDefaultProjectileSpawnOffsetY) -- see EnemyEntity::_hitbox_offset_y.
        double hitbox_offset_y{ 0.0 };
        double spawn_x{};                              // Original spawn X, kept for future leash/return-to-post AI; independent of the live, moving pos.x
        double move_speed_px_per_sec{};                // Already includes the spawn-time speed scale
        std::int32_t facing{ 1 };                       // +1 = moving right, -1 = moving left
        // AnimationStatePlayer snapshot. Only structurally sanity-checked here
        // (IsValid() has no EnemyAnimationDef to check bounds against) -- the
        // real check happens in EnemyEntity::RestoreState(), which does have one.
        std::int32_t anim_state_index{ 0 };
        std::int32_t anim_frame_index{ 0 };
        std::int32_t anim_frame_elapsed{ 0 };
        std::int32_t anim_state_elapsed{ 0 };
        // Already includes the spawn-time gravity_scale (same idea as
        // move_speed_px_per_sec above -- the scale itself isn't needed again
        // once these are computed).
        double gravity_per_frame{ 0.0 };
        double terminal_velocity_per_frame{ 0.0 };
        // Live SimpleGravityBody snapshot.
        double gravity_vel_y{ 0.0 };
        bool on_ground{ false };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    // Basic data-driven enemy: same shape for every EnemyKind at this stage --
    // per-kind differences are constructor parameters, not subclasses.
    class EnemyEntity final :
        public EntityBase,
        public systems::physics::ICollider,
        public systems::combat::IDamageable,
        public systems::physics::IDeflector,
        public systems::physics::IAttackInfo
    {
        // NOTE: no `Layer` alias here -- it would collide with ICollider::Layer() below.
        using RectF = foundation::math::RectF;
        using Vec2 = foundation::math::Vec2;
        using CollisionLayer = systems::physics::CollisionLayer;
        using TileAttribute = systems::physics::TileAttribute;
        using DamageTable = systems::combat::DamageTable;

    public:
        static constexpr std::uint16_t kStateVersion{ 1 };
        // Base horizontal speed (px/sec at scale=1.0) for whatever locomotion
        // the current animation state allows (e.g. Met's hop toward the player
        // during its "jump" state). Not yet per-kind; a placeholder until real
        // per-kind movement AI lands.
        static constexpr double kDefaultPatrolSpeedPxPerSec{ 20.0 };
        // Same per-frame values as PlayerTuning's normal (non-underwater)
        // gravity/terminalVelocity -- see SimpleGravityBody.h for why this is a
        // separate, independent constant rather than a shared reference to
        // PlayerTuning itself.
        static constexpr double kDefaultGravityPerFrame{ 0.25 };
        static constexpr double kDefaultTerminalVelocityPerFrame{ 19.0 };
        // Shared across every shooting enemy for now (see AnimationTransition::
        // projectile_spawns) -- revisit as a per-spawn parameter if a second
        // kind needs a different value.
        static constexpr int kDefaultProjectilePower{ 1 };
        // Contact damage (touching this enemy's body directly, no projectile
        // involved). Shared across every kind for now, same reasoning as
        // kDefaultProjectilePower above -- revisit as a per-spawn parameter if
        // a second kind needs a different value. Only actually dealt while
        // NOT deflecting -- see AttackPower().
        static constexpr int kDefaultContactDamagePower{ 1 };
        // The ring sprite's actual opaque pixels only cover an 8x8 area centered
        // in its 16x16 tile (ENEMY_PROJECTILES_N0_ALL_PATTERN.png); {3,3} (a 6x6
        // box) covered 75% of that, far stricter than the player's own Rock
        // Buster pellet (a 4x4 box against an ~11x6 visible sprite -- see
        // AttackActionState::AttackTuning::normalHitHalfSize). Matched to the
        // Buster's box size so an enemy shot is at least as forgiving to the
        // player as the player's own shot is to enemies.
        static constexpr Vec2 kDefaultProjectileHitHalfSize{ 2.0, 2.0 };
        // Muzzle offset from pos, applied before facing/angle rotation (Y only
        // for now -- positive = down, since screen Y grows downward). Tuning
        // knob for "the shot spawns a bit above/below the sprite's visual
        // center"; revisit as a per-spawn parameter if a second kind needs a
        // different value (same reasoning as kDefaultProjectilePower above).
        static constexpr double kDefaultProjectileSpawnOffsetY{ 5.0 };

        // Fresh placement (level/test spawn).
        // `palette_preset_index`: which of the kind's EnemyDefinition::
        // palette_presets `sprite_id` was already recolored to match (purely
        // for save-round-tripping -- rendering just uses `sprite_id` as-is).
        // `animation_def`: this kind's animation state graph, from
        // EnemyDefinitionCatalog; owned externally, must outlive this entity.
        // A null def degrades gracefully to always showing tile 0.
        // `toughness`: 0 = invincible, N = dies after N hits' worth of
        // accumulated normal-shot power (1 = one-hit kill).
        // `facing_texture_offset_left`: added to the animator's current tile
        // while facing left, for sheets with separate mirrored tiles (0 if
        // the sheet has none, or the art is symmetric).
        // `move_speed_scale`: multiplies kDefaultPatrolSpeedPxPerSec.
        // `gravity_scale`: multiplies both kDefaultGravityPerFrame and
        // kDefaultTerminalVelocityPerFrame, so heavier/floatier enemies can
        // fall faster/slower than the player without a second knob.
        // `hitbox_half_size`: entity-collision box (ICollider::Bounds()),
        // independent of `half_size` (render/probe footprint) -- see _hitbox_half.
        // `hitbox_offset_y`: shifts that box's center down from pos (0.0 for
        // kinds whose silhouette is already centered in the tile) -- see
        // _hitbox_offset_y.
        // `projectile_sprite_id`: sprite for shots this enemy's animation graph
        // fires (see AnimationTransition::projectile_spawns); default (-1) is
        // fine for enemies whose graph never carries projectile_spawns.
        EnemyEntity(
            EnemyKind kind,
            Vec2 spawn_pos,
            rendering::sprite::SpriteManager::Id sprite_id,
            int palette_preset_index,
            const animation::EnemyAnimationDef* animation_def,
            int facing_texture_offset_left,
            int toughness,
            Vec2 half_size,
            Vec2 hitbox_half_size,
            double hitbox_offset_y = 0.0,
            double move_speed_scale = 1.0,
            double gravity_scale = 1.0,
            rendering::sprite::SpriteManager::Id projectile_sprite_id =
                static_cast<rendering::sprite::SpriteManager::Id>(-1));
        // Reconstruction from saved state (kind/toughness/etc. all come from
        // `state`). `animation_def` is resolved externally the same way as
        // `sprite_id` -- by `state.kind`, via EnemyDefinitionCatalog; same for
        // `projectile_sprite_id`.
        EnemyEntity(
            const EnemyEntityState& state,
            rendering::sprite::SpriteManager::Id sprite_id,
            const animation::EnemyAnimationDef* animation_def,
            rendering::sprite::SpriteManager::Id projectile_sprite_id =
                static_cast<rendering::sprite::SpriteManager::Id>(-1));

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Main update (IUpdatable) -- faces the player, moves per the current
        // animation state's own rules, and steps the animation state machine
        // one tick
        void Update(const systems::view::ViewState* view, double dt) override;
        // Render (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::Enemy; }
        [[nodiscard]] std::uint16_t StateComponentVersion() const noexcept override { return kStateVersion; }
        bool SaveState(core::save::StateWriter& writer) const override;
        [[nodiscard]] EnemyEntityState CaptureState() const noexcept;
        bool RestoreState(const EnemyEntityState& state) noexcept;

        [[nodiscard]] EnemyKind Kind() const noexcept { return _kind; }

        // Wired externally after construction/restoration (mirrors PlayerEntity::
        // SetTerrainProbe()) -- not part of saved state. Without it, Update()
        // simply skips gravity for the frame (see SimpleGravityBody::Tick()).
        void SetTerrainProbe(const systems::physics::ITerrainProbe* terrain) noexcept { _terrain = terrain; }
        [[nodiscard]] bool IsOnGround() const noexcept { return _gravity.IsOnGround(); }

        // Wired externally once per frame, before Update() (see
        // AbstractActionPhase::updateActive_(), which calls this on every alive
        // EnemyEntity right before EntityManager::UpdateAll()) -- not part of
        // saved state. Feeds AnimationCondition::PlayerNear; without a call
        // this frame, PlayerNear transitions simply never fire (see
        // AnimationConditionInputs' sentinel default).
        void SetPlayerPosition(const Vec2& player_pos) noexcept
        {
            _player_pos = player_pos;
            _has_player_pos = true;
        }

        // Wired externally once per frame, before Update() (same call site as
        // SetPlayerPosition() above) -- not part of saved state. The real,
        // save-stated counter lives in AbstractActionPhase; this is just this
        // frame's read-only snapshot of it, for AnimationCondition parity
        // gating (see AnimationConditionInputs::shared_counter).
        void SetSharedAttackCounter(std::uint64_t value) noexcept { _shared_attack_counter = value; }

        // Drains and returns any projectile spawns queued during the last
        // Update() (see AnimationTransition::projectile_spawns). Polled by
        // AbstractActionPhase after updating all entities -- EnemyEntity itself
        // has no EntityManager access to spawn them directly.
        [[nodiscard]] std::vector<common::SpawnProjectileCommand> ConsumePendingProjectileSpawns() noexcept
        {
            return std::exchange(_pending_projectile_spawns, {});
        }

        // Drains and returns whether the last Update() fired a transition with
        // increments_shared_counter=true. Polled by AbstractActionPhase after
        // updating all entities, the same way as ConsumePendingProjectileSpawns()
        // above -- this entity has no access to the authoritative counter to
        // bump it itself.
        [[nodiscard]] bool ConsumeAttackCounterIncrementRequest() noexcept
        {
            return std::exchange(_pending_counter_increment_requested, false);
        }

        // ICollider
        IEntity& OwnerEntity() noexcept override { return *this; }
        const IEntity& OwnerEntity() const noexcept override { return *this; }
        RectF Bounds() const override;
        bool IsCollidable() const noexcept override { return IsAlive(); }
        CollisionLayer Layer() const noexcept override { return CollisionLayer::Enemy; }
        void OnTileCollision(const Vec2& normal, TileAttribute attr) override { (void)normal; (void)attr; }
        void OnEntityCollision(IEntity& other) override;

        // IDamageable
        bool ApplyAttack(const systems::physics::IAttackInfo& attack) noexcept override;
        [[nodiscard]] int CurrentHP() const noexcept override { return _health.CurrentHP(); }
        [[nodiscard]] int MaxHP() const noexcept override { return _health.MaxHP(); }
        [[nodiscard]] bool IsDead() const noexcept override { return _health.IsDead(); }

        // IDeflector -- data-driven per animation state, see AnimationState::deflects_attacks
        [[nodiscard]] bool DeflectsAttacks() const noexcept override { return _animator.DeflectsAttacks(); }

        // IAttackInfo -- contact damage (touching this enemy's body directly).
        // Returns 0 (no damage; see DamageTable::ComputeDamage()) while
        // deflecting, so a helmeted Met is safe to stand against, same as it
        // being immune to the Buster in that state.
        [[nodiscard]] int AttackPower() const noexcept override
        {
            return DeflectsAttacks() ? 0 : kDefaultContactDamagePower;
        }
        [[nodiscard]] systems::physics::WeaponId Weapon() const noexcept override
        {
            return systems::physics::WeaponId::EnemyContact;
        }

    private:
        EnemyKind _kind{ EnemyKind::Met };
        rendering::sprite::SpriteManager::Id _id{};    // Object sprite id (already the correct color/recolor)
        int _palette_preset_index{ 0 };                 // Which preset produced _id (save-only, see EnemyEntityState)
        int _facing_texture_offset_left{ 0 };           // Added to the animator's tile while facing left
        Vec2 _half{ 8.0, 8.0 };                         // Half-size of the sprite tile (render anchor) -- NOT the hit box, see _hitbox_half
        // Half-size of the entity-collision box (ICollider::Bounds()),
        // independent of _half -- same reasoning as ProjectileEntity's
        // _hit_half_size / PlayerEntity's _hitbox_half: the sprite's opaque
        // pixels don't fill the full tile, so using _half here let attacks
        // register while still visibly outside the sprite.
        Vec2 _hitbox_half{ 8.0, 8.0 };
        // Shifts the hit box's center down (+) from pos, Y only for now (same
        // reasoning as kDefaultProjectileSpawnOffsetY). Met's own silhouette
        // sits low in its tile and consistently bottom-anchored across every
        // pose while its top varies a lot (crouched under its helmet vs.
        // reared back mid-attack) -- a plain symmetric half-size around pos
        // can't track that, so this nudges the box toward the bottom instead
        // of centering it, trimming how far the box reaches above Met's
        // actual visible top.
        double _hitbox_offset_y{ 0.0 };
        systems::combat::HealthComponent _health{};     // HP + per-weapon resistance
        int _toughness{ 1 };                            // Raw tuning value driving _health's (max_hp, table); saved as-is
        animation::AnimationStatePlayer _animator{};    // Drives which tile is currently shown

        double _spawn_x{ 0.0 };                         // Original spawn X (unused for behavior currently; see EnemyEntityState::spawn_x)
        double _move_speed{ 0.0 };                      // px/sec, already includes the spawn-time scale
        int _facing{ 1 };                                // +1 right, -1 left; tracks the player every frame once SetPlayerPosition() has been called (see Update())

        systems::physics::SimpleGravityBody _gravity{};  // Vertical physics (see SetTerrainProbe())
        const systems::physics::ITerrainProbe* _terrain{ nullptr }; // Not saved; re-wired externally

        rendering::sprite::SpriteManager::Id _projectile_sprite_id{ static_cast<rendering::sprite::SpriteManager::Id>(-1) };
        std::vector<common::SpawnProjectileCommand> _pending_projectile_spawns{}; // Drained each frame; not saved
        bool _pending_counter_increment_requested{ false }; // Drained each frame; not saved -- see ConsumeAttackCounterIncrementRequest()

        Vec2 _player_pos{};        // See SetPlayerPosition(); not saved, re-wired externally every frame
        bool _has_player_pos{ false };
        std::uint64_t _shared_attack_counter{ 0 }; // See SetSharedAttackCounter(); not saved, re-wired externally every frame
    };
}
