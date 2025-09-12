//==============================================================================
// 
//  Project: mm2hack
//  IGameContext.h
// 
//  Interface for game context management.
// 
//==============================================================================
#pragma once

#include "apps/supervisor/ResourceManager.h"
#include "core/assembly/ISnapshotProvider.h"
#include "core/assembly/ITimeController.h"
#include "core/assembly/IWatchRegistry.h"
#include "core/assembly/StateProvider.h"

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
        // Get time contoller for managing game time
        virtual core::assembly::ITimeController& Time() = 0;
        // Retrieves the instance of the joystick manager for handling gamepad inputs
        virtual core::assembly::StateProvider& Input() = 0;
        // Retrieves the instance of the snapshot provider for capturing game state snapshots
        virtual core::assembly::ISnapshotProvider* Snapshot() = 0;
        // Retrieves the instance of the watch registry for monitoring game variables
        virtual core::assembly::IWatchRegistry& Watch() = 0;
        // Checks if the game context has been initialized
        virtual bool IsInitialized() const = 0;
    };
}