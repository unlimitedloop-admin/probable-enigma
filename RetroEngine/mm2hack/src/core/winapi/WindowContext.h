//==============================================================================
// 
//  Project: mm2hack
//  WindowContext.h
// 
//  The context for the WindowManager and GameLoopManager to share information.
// 
//==============================================================================
#pragma once

#include <Windows.h>

namespace mm2hack::core::winapi
{
    // Context structure for the main window, containing the window handle, viewer rate, and screen handle
    // This structure is used to manage the state of the main window in the application
    struct WindowContext
    {
        HWND hWnd = nullptr;
        float& viewerRate;
        int& screenHandle;
    };
}