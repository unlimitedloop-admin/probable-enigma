//==============================================================================
// 
//  Project: mm2hack
//  PauseManager.h
//
//  Manages the pause state and overlay for the game.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::core::overlay
{
    // Defines a class to manage the pause state and overlay in the game
    class PauseManager
    {
    public:
        PauseManager() = delete;
        ~PauseManager() = delete;
        PauseManager(const PauseManager&) = delete;
        PauseManager& operator=(const PauseManager&) = delete;
        PauseManager(PauseManager&&) = delete;
        PauseManager& operator=(PauseManager&&) = delete;
        // This class is not copyable or movable (static member defined only)

        // Set the pause state of the game
        static void SetPaused(bool paused);
        // Check if the game is currently paused
        static bool IsPaused();
        // Turn the pause state on or off
        static void Toggle();
        // Draw the pause overlay on the screen
        static void DrawOverlay();

    private:
        static inline std::wstring kClassName = L"PauseManager";

        static inline bool _isPaused = false;       // Static member to hold the pause state (default is not paused)
    };
}