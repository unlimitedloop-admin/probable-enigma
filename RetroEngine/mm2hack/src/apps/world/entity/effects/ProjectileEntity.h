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
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
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

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    // Entity representing a projectile (bullet, shot, etc.)
    class ProjectileEntity final : public EntityBase
    {
        using Layer = systems::view::Layer;

    public:
        static constexpr std::uint16_t kStateVersion{ 1 };

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

    private:
        Layer _draw_layer{ Layer::Actors };             // Drawing layer
        foundation::math::Vec2 _half{};                 // Half-size of the bounding box

        rendering::sprite::SpriteManager::Id _id{};     // Object sprite id
        int _base_texture{ 0 };                         // Base texture index
        common::ProjectileVisual _visual{ common::ProjectileVisual::Normal };
        std::int32_t _anim_frames{ 1 };                 // Animation frames
        double _anim_fps{ 0.0 };                        // Animation frames per second

        double _life_sec{ 1.0 };                        // Lifetime in seconds
        double _age_sec{ 0.0 };                         // Age in seconds
        std::uint32_t _elapsed_ticks{ 0 };
    };
}
