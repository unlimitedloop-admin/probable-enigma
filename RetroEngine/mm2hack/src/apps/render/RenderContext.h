//==============================================================================
// 
//  Project: mm2hack
//  RenderContext.h
// 
//  Governs the overall context for interpreting rendering operations.
// 
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::mod
{
    struct ViewState;
}

namespace mm2hack::apps::render
{
    enum class Layer : std::uint8_t
    {
        Background,
        Actors,
        Effects,
        Overlay
    };

    // Context for rendering operations
    struct RenderContext
    {
        const mod::ViewState* view{};
        Layer layer{ Layer::Actors };
    };
}