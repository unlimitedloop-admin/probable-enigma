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

#include <cassert>
#include <memory>
#include <string>
#include "apps/resources/ResourceManager.h"
#include "input/JoystickManager.h"

namespace mm2hack::core
{
    namespace assembly
    {
        class ITimeController;
        class StateProvider;
        class ISnapshotProvider;
    }
    namespace diagnostics
    {
        class IWatchRegistry;
    }
}



namespace mm2hack::apps::runtime
{
    // GameContext class that implements IGameContext, providing access to game resources and input management
    class GameContext final : public IGameContext
    {
        using ResourceManager   = apps::resources::ResourceManager;
        using ITimeController   = core::assembly::ITimeController;
        using StateProvider     = core::assembly::StateProvider;
        using ISnapshotProvider = core::assembly::ISnapshotProvider;
        using IWatchRegistry    = core::diagnostics::IWatchRegistry;
        using JoystickManager   = input::JoystickManager;

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
        ResourceManager& GetResourceManager() override { return *_resourceManager; }
        ResourceManager* GetResourceManagerPtr() override { return _resourceManager.get(); }
        ITimeController& Time() override { assert(_time); return *_time; }
        ITimeController* TryTime() { return _time; }    // Fail safe version
        StateProvider& Input() override { assert(_input); return *_input; }
        StateProvider* TryInput() { return _input; }    // Fail safe version
        ISnapshotProvider* Snapshot() override { return _snapshot; }
        IWatchRegistry& Watch() override;

        // Retrieves the instance of the joystick manager for handling gamepad input setup
        JoystickManager& Joystick() { return *_joystickManager; }
        const JoystickManager& Joystick() const { return *_joystickManager; }

        // Initializes the game context, setting up resources and input management
        void Initialize();
        // Shuts down the game context, releasing resources and cleaning up
        void Shutdown();
        // Checks if the game context has been initialized
        bool IsInitialized() const override;
        // Checks if the game context has been shut down
        bool IsShutdown() const { return !IsInitialized(); }

        // Attaches external services to the game context, such as time controller, input state provider, snapshot provider, and joystick manager
        void AttachServices(ITimeController* time, StateProvider* input, ISnapshotProvider* snapshot = nullptr, IWatchRegistry* watch = nullptr) noexcept;

    private:
        GameContext() = default;
        ~GameContext() = default;

    private:
        const std::wstring kClassName = L"GameContext";

        std::unique_ptr<ResourceManager> _resourceManager;  // SpriteBank, BGTiles, SoundDriver, etc.
        std::unique_ptr<JoystickManager> _joystickManager;  // Joystick manager for handling gamepad inputs (using only for setup)
        ITimeController* _time{ nullptr };                  // Time controller for managing game time
        StateProvider* _input{ nullptr };                   // Input state provider for handling user input
        ISnapshotProvider* _snapshot{ nullptr };            // Snapshot provider for capturing game state snapshots (optional)
        IWatchRegistry* _watch{ nullptr };                  // Watch registry for monitoring game variables (optional)
    };
}