//==============================================================================
// 
//  Project: mm2hack
//  IRenderable.h
// 
//  TODO: Add description.
// 
//==============================================================================
#pragma once

#include "apps/systems/view/RenderContext.h"

namespace mm2hack::apps::world::entity
{
    // Interface for renderable entities
    struct IRenderable
    {
        virtual ~IRenderable() = default;
        virtual systems::view::Layer DrawLayer() const noexcept = 0;
        virtual void Render(systems::view::RenderContext& ctx) = 0;
    };
}