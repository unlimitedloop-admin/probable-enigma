#include "WindowManager.h"

#include <DxLib.h>
#include <string>
#include <Windows.h>
#include "../resource.h"
#include "config/EnvironmentConfig.h"
#include "config/SystemConfig.h"
#include "utils/LogWriter.h"
#include "WindowMessageHandlers.h"

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

        if (config::EnvironmentConfig::GetBool(L"OUTPUT_LOG_ENABLE"))
        {
            DxLib::SetApplicationLogSaveDirectory(config::SystemConfig::kLogFilePath.c_str());
            DxLib::SetApplicationLogFileName(config::SystemConfig::kDxLibLogFileName.c_str());
            DxLib::SetOutApplicationLogValidFlag(TRUE);
            utils::LogWriter::Initialize(config::SystemConfig::kLogFilePath);
        }
        else
        {
            DxLib::SetOutApplicationLogValidFlag(FALSE);
        }

        const BOOL isAlwaysRun = config::EnvironmentConfig::GetBool(L"WINDOW_ALWAYS_RUN_ENABLE") ? TRUE : FALSE;
        DxLib::SetAlwaysRunFlag(isAlwaysRun);

        // Configure the window
        if (DxLib::SetDoubleStartValidFlag(FALSE) ||
            DxLib::ChangeWindowMode(TRUE) != DX_CHANGESCREEN_OK ||
            DxLib::SetMainWindowText(_windowTitle.c_str()) != 0 ||
            DxLib::SetWindowIconID(IDI_WNDICON) != 0 ||
            DxLib::LoadMenuResource(IDR_MAINMENU) != 0)
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

        _dxLibWnd = reinterpret_cast<WNDPROC>(GetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC));
        SetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));

        if (DxLib::SetDrawScreen(DX_SCREEN_BACK) == -1)
        {
            DxLib::DxLib_End();
            return false;
        }

        return true;
    }

    void WindowManager::RunMainLoop()
    {
        while (DxLib::ProcessMessage() == 0)
        {
            DxLib::ClearDrawScreen();
            // TODO: _sequence->OnExecute();
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

    HWND WindowManager::GetMainWindowHandle() const
    {
        return _mainWindowHandle;
    }

    bool WindowManager::IsMainWindowActive()
    {
        return static_cast<bool>(DxLib::GetWindowActiveFlag());
    }

    WNDPROC WindowManager::GetDxLibWnd() const
    {
        return _dxLibWnd;
    }

    LRESULT WindowManager::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)  
    {
        switch (message)
        {
        case WM_CREATE:
            HandleCreate(hwnd, lParam);
            return 0;
        case WM_DESTROY:
            HandleDestroy(hwnd);
            return 0;
        case WM_COMMAND:
            HandleCommand(hwnd, wParam);
            return 0;
        case WM_PAINT:
            HandlePaint(hwnd);
            return 0;
        case WM_SIZE:
            HandleSize(hwnd, wParam, lParam);
            return 0;
        case WM_KEYDOWN:
            HandleKeyDown(hwnd, wParam, lParam);
            return 0;
        case WM_KEYUP:
            HandleKeyUp(hwnd, wParam);
            return 0;
        default:
            return WindowManager::GetInstance().GetDxLibWnd() != nullptr
                ? CallWindowProc(WindowManager::GetInstance().GetDxLibWnd(), hwnd, message, wParam, lParam)
                : DefWindowProc(hwnd, message, wParam, lParam);
        }
    }
}