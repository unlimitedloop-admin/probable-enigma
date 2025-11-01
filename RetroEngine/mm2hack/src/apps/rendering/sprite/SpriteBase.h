//==============================================================================
// 
//  Project: mm2hack
//  SpriteBase.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "apps/graphics/entity/EntityBase.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/RenderContext.h"

namespace mm2hack::apps::rendering::sprite
{
    using entity::EntityBase;
    using foundation::math::Vec2;

    // Basic implementation of a sprite entity
    class SpriteBase : public EntityBase
    {
        using Layer = systems::view::Layer;

    public:
        Layer DrawLayer() const noexcept override { return _layer; }
        void SetLayer(Layer l) noexcept { _layer = l; }

        // Texture ID or graphic handle (e.g., DxLib graphic handle)
        int gfx{ -1 };
        Vec2 pivot{ 0.0, 0.0 }; // Base pivot point
        int  w{ 16 }, h{ 16 };
        int  z{ 0 };            // Simple sorting within the same layer

    protected:
        Layer _layer{ Layer::Actors };
    };
}