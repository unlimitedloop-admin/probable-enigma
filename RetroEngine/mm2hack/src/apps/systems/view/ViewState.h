//==============================================================================
// 
//  Project: mm2hack
//  ViewState.h
// 
//  View state management for the game.
// 
//==============================================================================
#pragma once

#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::view
{
    using Scalar = double;

    // Manages view state emulating NES VRAM
    struct ViewState
    {
        using conf = config::SystemConfig;

        Scalar camX{ 0.0 };
        Scalar camY{ 0.0 };
        double viewWorldX{};
        double viewWorldY{};
        int viewW{ conf::kScreenWidth };
        int viewH{ conf::kScreenHeight };
    };
}