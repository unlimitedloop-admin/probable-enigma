#include "WindowManager.h"

#include <DxLib.h>
#include <exception>
#include <string>
#include <Windows.h>
#include "../resource.h"
#include "config/EnvironmentConfig.h"
#include "config/SystemConfig.h"
#include "core/GameLoopManager.h"
#include "core/GameStateManager.h"
#include "exceptions/CoreException.h"
#include "exceptions/ErrorHandler.h"
#include "exceptions/ErrorLevel.h"
#include "utils/LogWriter.h"
#include "utils/string_converter.h"
#include "WindowMessageHandlers.h"

namespace mm2hack::core::winapi
{
    bool WindowManager::Initialize(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow, const std::wstring& windowTitle)
    {
        using namespace config;
        using namespace exceptions;
        using namespace utils;

        // Error reporting lambda function
        auto reportInitError = [](const std::wstring& message) -> bool
            {
                ErrorHandler::Handle(
                    message,
                    L"WindowManager",
                    L"Initialize",
                    ErrorLevel::Error
                );
                return false;
            };

        _hInstance = hInstance;
        if (windowTitle.empty())
        {
            return reportInitError(L"Window title is empty.");
        }
        _windowTitle = windowTitle;

        const bool isDebugMode =
            (lstrcmp(lpCmdLine, L"debug") == 0) ||
            EnvironmentConfig::GetBool(L"MM2HACK_DEBUG", false);

        if (EnvironmentConfig::GetBool(L"OUTPUT_LOG_ENABLE"))
        {
            using conf = config::SystemConfig;
            DxLib::SetApplicationLogSaveDirectory(conf::kLogFilePath.c_str());
            DxLib::SetApplicationLogFileName(conf::kDxLibLogFileName.c_str());
            DxLib::SetOutApplicationLogValidFlag(TRUE);
            LogWriter::Initialize(conf::kLogFilePath);
        }
        else
        {
            DxLib::SetOutApplicationLogValidFlag(FALSE);
        }

        const BOOL isAlwaysRun = EnvironmentConfig::GetBool(L"WINDOW_ALWAYS_RUN_ENABLE") ? TRUE : FALSE;
        DxLib::SetAlwaysRunFlag(isAlwaysRun);

        // Create the main window.
        if (DxLib::SetDoubleStartValidFlag(FALSE) ||
            DxLib::ChangeWindowMode(TRUE) != DX_CHANGESCREEN_OK ||
            DxLib::SetMainWindowText(_windowTitle.c_str()) != 0 ||
            DxLib::SetWindowIconID(IDI_WNDICON) != 0 ||
            DxLib::LoadMenuResource(IDR_MAINMENU) != 0)
        {
            return reportInitError(L"Failed to initialize the window.");
        }

        // DxLib initialization.
        if (DxLib::DxLib_Init() == -1)
        {
            return reportInitError(L"Failed to initialize DxLib.");
        }

        _mainWindowHandle = DxLib::GetMainWindowHandle();
        if (_mainWindowHandle == nullptr)
        {
            DxLib::DxLib_End();
            return reportInitError(L"Unable to obtain window handle.");
        }

        _dxLibWnd = reinterpret_cast<WNDPROC>(GetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC));
        SetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));

        if (DxLib::SetDrawScreen(DX_SCREEN_BACK) == -1)
        {
            DxLib::DxLib_End();
            return reportInitError(L"The SetDrawScreen(DX_SCREEN_BACK) function failed.");
        }

        if (!isDebugMode)
        {
            HMENU hMenu = GetMenu(_mainWindowHandle);
            EnableMenuItem(hMenu, ID_FILE_START_DEBUG, MF_BYCOMMAND | MF_GRAYED);
            DrawMenuBar(_mainWindowHandle);
        }

        return true;
    }

    void WindowManager::RunMainLoop()
    {
        GameLoopManager gameLoop(_mainWindowHandle, _viewerRate);
        gameLoop.Run();
    }

    void WindowManager::Shutdown()
    {
        DxLib::DxLib_End();
        _hInstance = nullptr;
        _mainWindowHandle = nullptr;
        _windowTitle.clear();
    }

    void WindowManager::UpdateMenuBarActivate() const
    {
        HMENU hMenu = GetMenu(_mainWindowHandle);
        if (hMenu == nullptr)
        {
            return;
        }

        UINT state = GameStateManager::GetInstance().CanActiveMenuBar()
            ? MF_ENABLED
            : MF_GRAYED;

        for (int i = 0; i <= 4; ++i)
        {
            EnableMenuItem(hMenu, i, MF_BYPOSITION | state);
        }

        DrawMenuBar(_mainWindowHandle);
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
        using namespace exceptions;

        auto ForwardToDefaultProc = [&]() -> LRESULT
            {
                auto dxWndProc = WindowManager::GetInstance().GetDxLibWnd();
                return dxWndProc != nullptr
                    ? CallWindowProc(dxWndProc, hwnd, message, wParam, lParam)
                    : DefWindowProc(hwnd, message, wParam, lParam);
            };

        try
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
                return ForwardToDefaultProc();
            }
        }
        catch (const CoreException& ex)
        {
            ErrorHandler::HandleEx(ex);
            return ForwardToDefaultProc();
        }
        catch (const std::exception& e)
        {
            ErrorHandler::Handle(utils::utf8_to_wstring(e.what()), L"WindowManager", L"WindowProc", ErrorLevel::FatalError);
            return ForwardToDefaultProc();
        }
    }
}