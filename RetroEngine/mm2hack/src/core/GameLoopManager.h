//==============================================================================
// 
//  Project: mm2hack
//  GameLoopManager.h
// 
//  Main game loop manager.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include <Windows.h>
#include "assembly/ITimeController.h"
#include "assembly/StateProvider.h"
#include "winapi/WindowContext.h"

namespace mm2hack::core
{
    // Class responsible for controlling the main game loop
    class GameLoopManager final
    {
        using ITimeController = assembly::ITimeController;
        using StateProvider = assembly::StateProvider;

    public:
        explicit GameLoopManager(winapi::WindowContext& context);

        // Start the main game loop
        void Run();

    private:
        const std::wstring kClassName = L"GameLoopManager";

        HWND _hWnd;                             // Reference to the client window handle
        float& _viewerRate;                     // Reference to the viewer rate
        int& _screenHandle;                     // Reference to the screen handle
        bool& _vSync;                           // Reference to the VSync enabled flag
        std::unique_ptr<StateProvider> _input;  // Input state provider for handling user input
        std::unique_ptr<ITimeController> _time; // Time controller for managing game time
    };
}