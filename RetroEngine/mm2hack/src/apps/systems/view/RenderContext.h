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

namespace mm2hack::apps::systems::view
{
    struct ViewState;

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
        const ViewState* view{};
        Layer layer{ Layer::Actors };
    };
}