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
        // Assumes the game is running: Retrieves the instance of the resource manager
        virtual supervisor::ResourceManager& GetResourceManager() = 0;
        // Get safety pointer to the ResourceManager, if not initialized, returns nullptr
        virtual supervisor::ResourceManager* GetResourceManagerPtr() = 0;
        // Assumes the game is running: Retrieves the instance of the joystick manager
        virtual input::JoystickManager& GetJoystickManager() = 0;
        virtual bool IsInitialized() const = 0;
    };
}