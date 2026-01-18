//==============================================================================
// 
//  Project: mm2hack
//  WindowMessageHandlers.h
// 
//  The window message call functions are split into separate files.
// 
//==============================================================================
#pragma once

#include <Windows.h>

namespace mm2hack::core::winapi
{
    // Handle the WM_USER_CREATE message
    void HandleCreate(HWND hWnd, LPARAM lParam);
    // Handle the WM_DESTROY message
    void HandleDestroy(HWND hWnd);
    // Handle the WM_COMMAND message
    void HandleCommand(HWND hWnd, WPARAM wParam);
    // Handle the WM_PAINT message
    void HandlePaint(HWND hWnd);
    // Handle the WM_SIZE message
    void HandleSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
    // Handle the WM_KEYDOWN message
    void HandleKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    // Handle the WM_KEYUP message
    void HandleKeyUp(HWND hWnd, WPARAM wParam);
}