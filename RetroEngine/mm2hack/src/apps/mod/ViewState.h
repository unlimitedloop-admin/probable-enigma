//==============================================================================
// 
//  Project: mm2hack
//  ViewState.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "config/SystemConfig.h"

namespace mm2hack::apps::mod
{
    // Manages view state emulating NES VRAM
    struct ViewState
    {
        double camX{ 0.0 };
        double camY{ 0.0 };
        int viewW{ config::SystemConfig::kScreenWidth };
        int viewH{ config::SystemConfig::kScreenHeight };
    };
}