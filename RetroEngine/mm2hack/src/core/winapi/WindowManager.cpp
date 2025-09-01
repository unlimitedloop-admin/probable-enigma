#include "pch.h"

#include "WindowManager.h"

#include <exception>
#include "../resource.h"
#include "apps/NES/NESPalette.h"
#include "config/ConfigUIManager.h"
#include "config/EnvironmentConfig.h"
#include "config/HudConfig.h"
#include "config/SystemConfig.h"
#include "core/GameLoopManager.h"
#include "core/GameStateManager.h"
#include "core/ui/SettingsWindow.h"
#include "exceptions/CoreException.h"
#include "exceptions/ErrorHandler.h"
#include "exceptions/ErrorLevel.h"
#include "utils/LogWriter.h"
#include "utils/string_converter.h"
#include "WindowContext.h"
#include "WindowMessageHandlers.h"

namespace mm2hack::core::winapi
{
    constexpr UINT WM_USER_CREATE = WM_USER + 100;

    bool WindowManager::Initialize(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow, const std::wstring& windowTitle)
    {
        using namespace config;
        using namespace exceptions;
        using namespace utils;
        using conf = config::SystemConfig;

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
        _viewerRate = conf::kScreenScale;

        if (EnvironmentConfig::GetBool(L"OUTPUT_LOG_ENABLE"))
        {
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
        if (DxLib::SetDoubleStartValidFlag(FALSE) != 0 ||
            DxLib::SetAlwaysRunFlag(FALSE) != 0 ||
            DxLib::SetUseASyncChangeWindowModeFunction(FALSE, nullptr, nullptr) != 0 ||
            DxLib::SetWindowUserCloseEnableFlag(TRUE) != 0 ||
            DxLib::SetDxLibEndPostQuitMessageFlag(TRUE) != 0 ||
            DxLib::ChangeWindowMode(TRUE) != DX_CHANGESCREEN_OK ||
            DxLib::SetGraphMode(
                static_cast<int>(conf::kScreenWidth * conf::kScreenScaleMax),
                static_cast<int>(conf::kScreenHeight * conf::kScreenScaleMax),
                conf::kScreenColorDepth) != 0 ||
            DxLib::SetWindowSizeChangeEnableFlag(FALSE, FALSE) != 0 ||
            DxLib::SetWindowSize(
                static_cast<int>(conf::kScreenWidth * _viewerRate),
                static_cast<int>(conf::kScreenHeight * _viewerRate)) != 0 ||
            DxLib::SetWindowSizeExtendRate(1.0f) != 0 ||
            DxLib::SetMainWindowText(_windowTitle.c_str()) != 0 ||
            DxLib::SetWindowIconID(IDI_WNDICON) != 0 ||
            DxLib::LoadMenuResource(IDR_MAINMENU) != 0 ||
            DxLib::SetWindowInitPosition(0, 0) != 0)
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

        // Register the settings window class.
        INITCOMMONCONTROLSEX iccex{};
        iccex.dwSize = sizeof(iccex);
        iccex.dwICC = ICC_WIN95_CLASSES;
        InitCommonControlsEx(&iccex);
        overlay::SettingsWindow::RegisterWindowClass(hInstance);

        _mainWindowHandle = DxLib::GetMainWindowHandle();
        if (_mainWindowHandle == nullptr)
        {
            DxLib::DxLib_End();
            return reportInitError(L"Unable to obtain window handle.");
        }

        _dxLibWnd = reinterpret_cast<WNDPROC>(GetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC));
        SetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));
        PostMessage(_mainWindowHandle, WM_USER_CREATE, 0, 0);

        // Set the background color using NES palette
        if (!apps::NES::NESPalette::LoadPaletteFromFile(conf::kNESPaletteFilepath))
        {
            // If palette loading fails, handle the error
            DxLib::DxLib_End();
            return reportInitError(L"Failed to load NES palette.");
        }
        apps::NES::NESPalette::SetBackgroundFor(conf::kDefaultNESPaletteIndex);
        DxLib::ChangeFont(L"Segoe UI");

        _screenHandle = DxLib::MakeScreen(conf::kScreenWidth, conf::kScreenHeight, FALSE);  // Create a screen for drawing
        if (_screenHandle == -1)
        {
            DxLib::DxLib_End();
            return reportInitError(L"The SetDrawScreen(DX_SCREEN_BACK) function failed.");
        }

        // Load the HUD configuration from the ini file.
        HudConfig hudConfig;
        ConfigUIManager::LoadHudConfig(hudConfig);

        return true;
    }

    void WindowManager::RunMainLoop()
    {
        WindowContext context{
            .hWnd = _mainWindowHandle,
            .viewerRate = _viewerRate,
            .screenHandle = _screenHandle
        };

        GameLoopManager gameLoop(context);
        gameLoop.Run();
    }

    void WindowManager::Shutdown()
    {
        DxLib::DxLib_End();
        _hInstance = nullptr;
        _mainWindowHandle = nullptr;
        _windowTitle.clear();
    }

    bool WindowManager::ChangeWindowSize(float viewerRate)
    {
        using conf = config::SystemConfig;

        // Validate the viewer rate.
        int clientW = static_cast<int>(conf::kScreenWidth * viewerRate);
        int clientH = static_cast<int>(conf::kScreenHeight * viewerRate);

        HWND hWnd = GetMainWindowHandle();
        DWORD style = GetWindowLong(hWnd, GWL_STYLE);
        DWORD exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        HMENU hMenu = GetMenu(hWnd);

        // Adjust the window size based on the client area size and styles.
        RECT rect = { 0, 0, clientW, clientH };
        AdjustWindowRectEx(&rect, style, hMenu != nullptr, exStyle);
        int windowW = rect.right - rect.left;
        int windowH = rect.bottom - rect.top;

        // Correction: Check the actual client size and add height if there is any discrepancy.
        SetWindowPos(hWnd, nullptr, 0, 0, windowW, windowH, SWP_NOMOVE | SWP_NOZORDER);
        ShowWindow(hWnd, SW_SHOW);

        RECT actualClient = {};
        GetClientRect(hWnd, &actualClient);
        int actualClientH = actualClient.bottom - actualClient.top;

        if (actualClientH < clientH)
        {
            int delta = clientH - actualClientH;
            windowH += delta;

            SetWindowPos(hWnd, nullptr, 0, 0, windowW, windowH, SWP_NOMOVE | SWP_NOZORDER);
        }
        _viewerRate = viewerRate;

        return true;
    }

    int WindowManager::GetScreenWidth() const
    {
        return static_cast<int>(config::SystemConfig::kScreenWidth * _viewerRate);
    }

    int WindowManager::GetScreenHeight() const
    {
        return static_cast<int>(config::SystemConfig::kScreenHeight * _viewerRate);
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

    int WindowManager::GetScreenHandle() const
    {
        return _screenHandle;
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
                break;
            case WM_DESTROY:
                HandleDestroy(hwnd);
                break;
            case WM_COMMAND:
                HandleCommand(hwnd, wParam);
                break;
            case WM_PAINT:
                HandlePaint(hwnd);
                break;
            case WM_SIZE:
                HandleSize(hwnd, wParam, lParam);
                break;
            case WM_KEYDOWN:
                HandleKeyDown(hwnd, wParam, lParam);
                break;
            case WM_KEYUP:
                HandleKeyUp(hwnd, wParam);
                break;
            default:
                break;
            }
            return ForwardToDefaultProc();
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