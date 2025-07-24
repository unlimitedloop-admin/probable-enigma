//==============================================================================
// 
//  Project: mm2hack
//  IGameContext.h
// 
//  Interface for game context management
// 
//==============================================================================
#pragma once

#include "apps/supervisor/ResourceManager.h"
#include "input/JoystickManager.h"

namespace mm2hack::apps::deal
{
    // Interface for game context management, providing a base for game-specific contexts
    class IGameContext
    {
    public:
        virtual ~IGameContext() = default;
        virtual supervisor::ResourceManager& GetResourceManager() = 0;
        virtual input::JoystickManager& GetJoystickManager() = 0;
    };
}