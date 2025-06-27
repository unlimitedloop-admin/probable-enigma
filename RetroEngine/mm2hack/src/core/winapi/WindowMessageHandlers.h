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
    void HandleCreate(HWND hWnd, LPARAM lParam);
    void HandleDestroy(HWND hWnd);
    void HandleCommand(HWND hWnd, WPARAM wParam);
    void HandlePaint(HWND hWnd);
    void HandleSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void HandleKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void HandleKeyUp(HWND hWnd, WPARAM wParam);
}