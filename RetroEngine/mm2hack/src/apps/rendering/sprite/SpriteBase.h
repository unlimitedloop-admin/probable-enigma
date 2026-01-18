//==============================================================================
// 
//  Project: mm2hack
//  SpriteBase.h
// 
//  Generative sprite entity base class.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/RenderContext.h"

namespace mm2hack::apps::rendering::sprite
{
    // Basic implementation of a sprite entity
    class SpriteBase : public world::entity::EntityBase
    {
        using Layer = systems::view::Layer;
        using Vec2 = foundation::math::Vec2;

    public:
        Layer DrawLayer() const noexcept override { return _layer; }
        void SetLayer(Layer l) noexcept { _layer = l; }

        // Texture ID or graphic handle (e.g., DxLib graphic handle)
        int gfx{ -1 };
        Vec2 pivot{ 0.0, 0.0 }; // Base pivot point
        int  w{ 16 }, h{ 16 };
        int  z{ 0 };            // Simple sorting within the same layer

    protected:
        Layer _layer{ Layer::Actors };  // Default layer
    };
}