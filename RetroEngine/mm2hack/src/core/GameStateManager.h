//==============================================================================
// 
//  Project: mm2hack
//  GameStateManager.h
// 
//  Manages the game state across different scenes in the game.
// 
//==============================================================================
#pragma once

#include <string>
#include "GameState.h"

namespace mm2hack::core
{
    // Manages the game state across different scenes in the game
    class GameStateManager final
    {
    public:
        static GameStateManager& GetInstance()
        {
            static GameStateManager instance;
            return instance;
        }

        GameStateManager(const GameStateManager&) = delete;
        GameStateManager& operator=(const GameStateManager&) = delete;
        GameStateManager(GameStateManager&&) = delete;
        GameStateManager& operator=(GameStateManager&&) = delete;
        // GameStateManager is a singleton, so we delete the copy and move constructors and assignment operators.

        // Set the current game state
        void SetState(GameState newState);
        // Get the current game state
        GameState GetState() const;
        // Check if the current game state matches the specified state
        bool Is(GameState state) const;
        // Gets whether the menu bar can be operated
        bool CanActiveMenuBar() const;
        // Check if the game is currently running
        bool IsRunning() const;

    private:
        GameStateManager() = default;
        ~GameStateManager() = default;

    private:
        const std::wstring kClassName{ L"GameStateManager" };

        GameState _currentState = GameState::Standby;   // The current game state
    };
}