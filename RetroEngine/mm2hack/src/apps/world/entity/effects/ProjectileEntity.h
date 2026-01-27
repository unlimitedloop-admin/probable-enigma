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
#include "apps/world/entity/common/SpawnProjectileCommand.h"

namespace mm2hack::apps::world::entity::effects
{
    // Entity representing a projectile (bullet, shot, etc.)
    class ProjectileEntity final : public EntityBase
    {
    public:
        explicit ProjectileEntity(const common::SpawnProjectileCommand& cmd);

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Main update (IUpdatable)
        void Update(double dt) override;
        // Render (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;

    private:
        systems::view::Layer _draw_layer{ systems::view::Layer::Actors };   // Drawing layer
        foundation::math::Vec2 _half{};                                     // Half-size of the bounding box

        rendering::sprite::SpriteManager::Id _id{};                         // Object sprite id
        int _base_texture{ 0 };             // Base texture index
        std::int32_t _anim_frames{ 1 };     // Animation frames
        double _anim_fps{ 0.0 };            // Animation frames per second

        double _life_sec{ 1.0 };            // Lifetime in seconds
        double _age_sec{ 0.0 };             // Age in seconds
    };
}