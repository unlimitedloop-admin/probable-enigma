//==============================================================================
// 
//  Project: mm2hack
//  GameContext.h
// 
//  GameContext class for managing the game state and resources.
// 
//==============================================================================
#pragma once

#include "IGameContext.h"

#include <memory>
#include "apps/supervisor/ResourceManager.h"
#include "input/JoystickManager.h"

namespace mm2hack::apps::deal
{
    // GameContext class that implements IGameContext, providing access to game resources and input management
    class GameContext final : public IGameContext
    {
    public:
        static GameContext& GetInstance()
        {
            static GameContext instance;
            return instance;
        }

        GameContext(const GameContext&) = delete;
        GameContext& operator=(const GameContext&) = delete;
        GameContext(GameContext&&) = delete;
        GameContext& operator=(GameContext&&) = delete;
        // GameContext is a singleton, so we delete the copy and move constructors and assignment operators.

        // Assumes the game is running: Retrieves the instance of the resource manager
        supervisor::ResourceManager& GetResourceManager() override { return *_resourceManager; }
        supervisor::ResourceManager* GetResourceManagerPtr() override { return _resourceManager.get(); }
        input::JoystickManager& GetJoystickManager() override { return *_joystickManager; }

        // Initializes the game context, setting up resources and input management
        void Initialize();
        // Shuts down the game context, releasing resources and cleaning up
        void Shutdown();
        // Checks if the game context has been initialized
        bool IsInitialized() const override;
        // Checks if the game context has been shut down
        bool IsShutdown() const { return !IsInitialized(); }


    private:
        GameContext() = default;
        ~GameContext() = default;

        std::unique_ptr<supervisor::ResourceManager> _resourceManager;  // SpriteBank, BGTiles, SoundDriver, etc.
        std::unique_ptr<input::JoystickManager> _joystickManager;       // Joystick input manager for handling gamepad inputs
    };
}