//==============================================================================
// 
//  Project: mm2hack
//  IRenderable.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "apps/render/RenderContext.h"

namespace mm2hack::apps::graphics::entity
{
    using render::Layer;
    using render::RenderContext;

    // Interface for renderable entities
    struct IRenderable
    {
        virtual ~IRenderable() = default;
        virtual Layer DrawLayer() const noexcept = 0;
        virtual void Render(RenderContext& ctx) = 0;
    };
}