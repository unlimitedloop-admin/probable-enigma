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

        supervisor::ResourceManager& GetResourceManager() override { return *_resourceManager; }
        input::JoystickManager& GetJoystickManager() override { return *_joystickManager; }

        void Initialize();
        void Shutdown();

    private:
        GameContext() = default;
        ~GameContext() = default;

        std::unique_ptr<supervisor::ResourceManager> _resourceManager;
        std::unique_ptr<input::JoystickManager> _joystickManager;
    };
}