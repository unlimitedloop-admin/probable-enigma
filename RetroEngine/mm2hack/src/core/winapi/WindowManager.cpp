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
    constexpr UINT WM_USER_CREATE = WM_USER + 100;

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

        InitializeMenuOnStartup();
        UpdateMenuBarState();

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
        PostMessage(_mainWindowHandle, WM_USER_CREATE, 0, 0);

        if (DxLib::SetDrawScreen(DX_SCREEN_BACK) == -1)
        {
            DxLib::DxLib_End();
            return reportInitError(L"The SetDrawScreen(DX_SCREEN_BACK) function failed.");
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

    void WindowManager::InitializeMenuOnStartup()
    {
        HMENU hMenu = GetMenu(_mainWindowHandle);
        if (!hMenu) return;

        if (!IsDebugMode())
        {
            // Debug(&D) is the 3rd command on the menu.
            RemoveMenu(hMenu, 3, MF_BYPOSITION);
            HMENU hFileMenu = GetSubMenu(hMenu, 0); // File menu is the first submenu
            if (hFileMenu)
            {
                // Remove the Debug Start command from the File menu.
                RemoveMenu(hFileMenu, ID_FILE_START_DEBUG, MF_BYCOMMAND);
            }
            DrawMenuBar(_mainWindowHandle);     // Update the menu
        }
    }

    void WindowManager::UpdateMenuBarState() const
    {
        HMENU hMenu = GetMenu(_mainWindowHandle);
        if (!hMenu) return;

        const bool canActivate = GameStateManager::GetInstance().CanActiveMenuBar();
        const int topCount = GetMenuItemCount(hMenu);

        for (int i = 0; i < topCount; ++i)
        {
            EnableMenuItem(hMenu, i, MF_BYPOSITION | (canActivate ? MF_ENABLED : MF_GRAYED));
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
            case WM_USER_CREATE:
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

    bool WindowManager::IsDebugMode() const
    {
        return (lstrcmp(GetCommandLine(), L"debug") == 0) ||
            config::EnvironmentConfig::GetBool(L"MM2HACK_DEBUG", false);
    }
}