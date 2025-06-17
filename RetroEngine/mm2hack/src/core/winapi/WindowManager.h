//==============================================================================
// 
//  Project: mm2hack
//  WindowManager.h
// 
//  It creates window and manages the main procedure that runs the game.
// 
//==============================================================================
#pragma once

#include <string>
#include <Windows.h>

namespace mm2hack::core::winapi
{
    // The management of the main window and window handles in the application
    class WindowManager final
    {
    public:
        static WindowManager& GetInstance()
        {
            static WindowManager instance;
            return instance;
        }

        WindowManager(const WindowManager&) = delete;
        WindowManager& operator=(const WindowManager&) = delete;
        WindowManager(WindowManager&&) = delete;
        WindowManager& operator=(WindowManager&&) = delete;
        // WindowManager is a singleton, so we delete the copy and move constructors and assignment operators.

        bool Initialize(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow, const std::wstring& windowTitle);
        void RunMainLoop();
        void Shutdown();
        HWND GetMainWindowHandle();
        bool IsMainWindowActive();

    private:
        WindowManager() = default;
        ~WindowManager() = default;

        HWND _mainWindowHandle;
        HINSTANCE _hInstance = nullptr;
        std::wstring _windowTitle;
    };
}