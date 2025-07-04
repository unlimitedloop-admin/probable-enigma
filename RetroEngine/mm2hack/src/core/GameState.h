//==============================================================================
// 
//  Project: mm2hack
//  GameState.h
// 
//  Enums representing the current state of the game.
// 
//==============================================================================
#pragma once

namespace mm2hack::core
{
    // Enum representing the current state of the game
    enum class GameState
    {
        Standby,        // When the game is not running, such as when the main menu is displayed.
        Running,        // When the game is running and active.
        Paused,         // When the game is paused.
        MenuActive,     // When a menu is active, such as the pause menu or options menu.
        Transitioning,  // When the game is transitioning between states, such as loading a new level or scene.
    };
}