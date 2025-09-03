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

        // Initialize the main window and DxLib
        bool Initialize(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow, const std::wstring& windowTitle);
        // Run the main loop of the application
        void RunMainLoop();
        // Shutdown the application and release resources
        void Shutdown();

        // Change the size of the main window based on the viewer rate
        bool ChangeWindowSize(float viewerRate);
        // Get the current screen width and height
        int GetScreenWidth() const;
        int GetScreenHeight() const;
        float GetViewerRate() const { return _viewerRate; }

        // Initialize the menu bar on application startup
        void InitializeMenuOnStartup();
        // Update the state of the menu bar based on the current game state
        void UpdateMenuBarState() const;

        // Get the main window handle
        HWND GetMainWindowHandle() const;
        // Check if the main window is currently active
        bool IsMainWindowActive();
        // Get drawing screen handle
        int GetScreenHandle() const;
        // Set VSync enabled/disabled
        bool SetVSyncEnabled(bool enabled);

        // Get the window procedure handle for DxLib
        WNDPROC GetDxLibWnd() const;

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    private:
        WindowManager() = default;
        ~WindowManager() = default;

        HWND _mainWindowHandle = nullptr;
        HINSTANCE _hInstance = nullptr;
        WNDPROC _dxLibWnd = nullptr;
        std::wstring _windowTitle;          // Title of the main window
        float _viewerRate = 0.0f;           // Viewer rate for the main window, used for scaling
        int _screenHandle = -1;             // Handle for the screen
        bool _vSync{ false };               // VSync enabled/disabled

        bool IsDebugMode() const;           // Check if the application is running in debug mode
        float LoadViewerRate() const;       // Load the viewer rate from configuration
        bool LoadVSync() const;             // Load the VSync setting from configuration

        void SyncWindowSizeMenuCheck(float viewerRate) const;
    };
}