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

namespace mm2hack::core
{
    // Class responsible for controlling the main game loop
    class GameLoopManager
    {
    public:
        GameLoopManager(HWND hWnd, const float& viewerRate);
        void Run();

    private:
        HWND _hWnd;
        const float& _viewerRate;
    };
}