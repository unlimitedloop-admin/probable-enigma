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
    class GameLoopManager
    {
    public:
        explicit GameLoopManager(winapi::WindowContext& context);
        void Run();

    private:
        HWND _hWnd;
        float& _viewerRate;
        int& _screenHandle;
        bool& _vSync;
    };
}