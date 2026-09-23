//==============================================================================
// 
//  Project: mm2hack
//  ProjectileEntity.h
// 
//  Collidable object such as bullets or shot within the game world.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <cstdint>
#include <utility>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/physics/CollisionLayer.h"
#include "apps/systems/physics/IAttackInfo.h"
#include "apps/systems/physics/ICollider.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "apps/world/entity/IEntity.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::effects
{
    struct ProjectileEntityState final
    {
        EntityKinematicState kinematic{};
        systems::view::Layer draw_layer{ systems::view::Layer::Effects };
        std::int32_t base_texture{};
        common::ProjectileVisual visual{ common::ProjectileVisual::Normal };
        std::int32_t animation_frames{ 1 };
        double animation_fps{};
        double lifetime_seconds{ 1.0 };
        double age_seconds{};
        std::uint32_t elapsed_ticks{};
        std::int32_t power{ 1 };
        foundation::math::Vec2 hit_half_size{ 2.0, 2.0 };
        systems::physics::CollisionLayer collision_layer{ systems::physics::CollisionLayer::ProjectilePlayer };
        systems::physics::WeaponId weapon{ systems::physics::WeaponId::Buster };
        bool terrain_collision_enabled{ false };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    // Entity representing a projectile (bullet, shot, etc.)
    class ProjectileEntity final : public EntityBase, public systems::physics::ICollider, public systems::physics::IAttackInfo
    {
        // NOTE: no `Layer` alias here -- it would collide with ICollider::Layer() below.
        using RectF = foundation::math::RectF;
        using Vec2 = foundation::math::Vec2;
        using CollisionLayer = systems::physics::CollisionLayer;
        using TileAttribute = systems::physics::TileAttribute;

    public:
        static constexpr std::uint16_t kStateVersion{ 3 };

        explicit ProjectileEntity(const common::SpawnProjectileCommand& cmd);
        ProjectileEntity(
            const ProjectileEntityState& state,
            rendering::sprite::SpriteManager::Id sprite_id);

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Main update (IUpdatable)
        void Update(const systems::view::ViewState* view, double dt) override;
        // Render (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::Projectile; }
        [[nodiscard]] std::uint16_t StateComponentVersion() const noexcept override { return kStateVersion; }
        bool SaveState(core::save::StateWriter& writer) const override;
        [[nodiscard]] ProjectileEntityState CaptureState() const noexcept;
        bool RestoreState(const ProjectileEntityState& state) noexcept;

        // Wired externally after construction/restoration (mirrors EnemyEntity::
        // SetTerrainProbe()) -- not part of saved state. Without it, Update()
        // simply skips terrain collision for the frame (the shot flies through walls).
        void SetTerrainProbe(const systems::physics::ITerrainProbe* terrain) noexcept { _terrain = terrain; }

        // ICollider
        IEntity& OwnerEntity() noexcept override { return *this; }
        const IEntity& OwnerEntity() const noexcept override { return *this; }
        RectF Bounds() const override;
        bool IsCollidable() const noexcept override { return IsAlive(); }
        CollisionLayer Layer() const noexcept override { return _collision_layer; }
        void OnTileCollision(const Vec2& normal, TileAttribute attr) override;
        void OnEntityCollision(IEntity& other) override;

        // Drains (and clears) whether this shot deflected off an
        // IDeflector-implementing collider since the last call. Polled by
        // AbstractActionPhase right after collision resolution to play
        // "defend_shot" once per frame -- this entity has no AudioManager
        // access of its own, same reasoning as every other entity-side SE.
        // Not saved (see CanCaptureState()-style reasoning elsewhere): this
        // flag never survives more than a fraction of a frame in practice.
        [[nodiscard]] bool ConsumeDeflected() noexcept
        {
            return std::exchange(_pending_deflected, false);
        }

        // IAttackInfo
        [[nodiscard]] int AttackPower() const noexcept override { return _power; }
        // A charged Rock Buster shot is still the Buster -- charge level only
        // changes AttackPower(), not the weapon (see SpawnProjectileCommand::weapon).
        [[nodiscard]] systems::physics::WeaponId Weapon() const noexcept override { return _weapon; }

    private:
        // Samples the terrain at the hit box's leading edge (in the direction of
        // travel) and reports whether it has run into a solid tile. No-op (false)
        // when no ITerrainProbe has been wired -- see SetTerrainProbe().
        [[nodiscard]] bool checkTerrainCollision_() const;
        // Redirects this shot's velocity off an IDeflector instead of dying --
        // keeps its speed, reverses horizontal direction (back the way it
        // came), and arcs it upward. Same graphic, new trajectory; no change
        // to collisionLayer/weapon, so it can still hit something else along
        // the new path. Also arms _deflect_grace_frames (see its declaration).
        void deflect_() noexcept;

    private:
        systems::view::Layer _draw_layer{ systems::view::Layer::Actors }; // Drawing layer
        foundation::math::Vec2 _half{};                 // Half-size of the sprite's draw footprint (rendering only)

        rendering::sprite::SpriteManager::Id _id{};     // Object sprite id
        int _base_texture{ 0 };                         // Base texture index
        common::ProjectileVisual _visual{ common::ProjectileVisual::Normal };
        std::int32_t _anim_frames{ 1 };                 // Animation frames
        double _anim_fps{ 0.0 };                        // Animation frames per second

        double _life_sec{ 1.0 };                        // Lifetime in seconds
        double _age_sec{ 0.0 };                         // Age in seconds
        std::uint32_t _elapsed_ticks{ 0 };

        int _power{ 1 };                                // Attack power carried on contact
        foundation::math::Vec2 _hit_half_size{ 2.0, 2.0 }; // Half-size of the attack hit judgement box (independent of _half)
        CollisionLayer _collision_layer{ CollisionLayer::ProjectilePlayer };
        systems::physics::WeaponId _weapon{ systems::physics::WeaponId::Buster };
        bool _terrain_collision_enabled{ false };       // Per-shot opt-in; see SpawnProjectileCommand::terrainCollisionEnabled

        const systems::physics::ITerrainProbe* _terrain{ nullptr }; // Not saved; re-wired externally
        bool _pending_deflected{ false };               // Not saved; see ConsumeDeflected()
        // Frames left to ignore further OnEntityCollision() calls after a
        // deflection. Without this, the shot -- still alive/collidable, still
        // overlapping the deflector's Bounds() for a few frames while it
        // drifts away at its (comparatively slow) redirected velocity -- gets
        // re-processed by CollisionSystem every one of those frames and
        // re-deflects the already-deflected velocity each time, reading as an
        // erratic S-curve. Not saved -- short-lived (a handful of frames),
        // same reasoning as _pending_deflected above.
        int _deflect_grace_frames{ 0 };
    };
}
