#include "WindowManager.h"

#include <DxLib.h>
#include <string>
#include <Windows.h>

namespace mm2hack::core::winapi
{
    bool WindowManager::Initialize(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow, const std::wstring& windowTitle)
    {
        if (hInstance == nullptr || lpCmdLine == nullptr || windowTitle.empty())
        {
            return false;
        }
        _hInstance = hInstance;
        _windowTitle = windowTitle;

        // Configure the window
        if (
            DxLib::ChangeWindowMode(TRUE) != DX_CHANGESCREEN_OK ||
            DxLib::SetMainWindowText(_windowTitle.c_str()) != 0
            )
        {
            return false;
        }

        if (DxLib::DxLib_Init() == -1)
        {
            return false;
        }

        _mainWindowHandle = DxLib::GetMainWindowHandle();
        if (_mainWindowHandle == nullptr)
        {
            DxLib::DxLib_End();
            return false;
        }

        DxLib::SetDrawScreen(DX_SCREEN_BACK);

        return true;
    }

    void WindowManager::RunMainLoop()
    {
        while (DxLib::ProcessMessage() == 0)
        {
            // Handle other game logic here
            // ...
            // Render the frame
            DxLib::ClearDrawScreen();
            DxLib::ScreenFlip();
        }
    }

    void WindowManager::Shutdown()
    {
        DxLib::DxLib_End();
        _hInstance = nullptr;
        _mainWindowHandle = nullptr;
        _windowTitle.clear();
    }
}