//==============================================================================
// 
//  Project: mm2hack
//  GameLoopManager.h
// 
//  Main game loop manager.
// 
//==============================================================================
#pragma once

#include <Windows.h>
#include "winapi/WindowContext.h"

namespace mm2hack::core
{
    // Class responsible for controlling the main game loop
    class GameLoopManager final
    {
    public:
        explicit GameLoopManager(winapi::WindowContext& context);

        // Start the main game loop
        void Run();

    private:
        HWND _hWnd;         // Reference to the client window handle
        float& _viewerRate; // Reference to the viewer rate
        int& _screenHandle; // Reference to the screen handle
        bool& _vSync;       // Reference to the VSync enabled flag
    };
}