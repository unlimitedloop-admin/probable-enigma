#include "GameStateManager.h"

#include "GameState.h"

namespace mm2hack::core
{
    GameStateManager& GameStateManager::GetInstance()
    {
        static GameStateManager instance;
        return instance;
    }

    void GameStateManager::SetState(GameState newState)
    {
        _currentState = newState;
    }

    GameState GameStateManager::GetState() const
    {
        return _currentState;
    }

    bool GameStateManager::Is(GameState state) const
    {
        return _currentState == state;
    }

    bool GameStateManager::CanActiveMenuBar() const
    {
        return _currentState == GameState::Paused ||
            _currentState == GameState::MenuActive ||
            _currentState == GameState::Standby;
    }
}