#include "WindowManager.h"

#include <DxLib.h>
#include <exception>
#include <string>
#include <Windows.h>
#include "../resource.h"
#include "config/EnvironmentConfig.h"
#include "config/SystemConfig.h"
#include "core/GameLoopManager.h"
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
        _hInstance = hInstance;
        if (windowTitle.empty())
        {
            exceptions::ErrorHandler::Handle(
                L"Window title is empty.",
                L"WindowManager",
                L"Initialize",
                exceptions::ErrorLevel::Error
            );
            return false;
        }
        _windowTitle = windowTitle;
        const bool isDebugMode =
            (lstrcmp(lpCmdLine, L"debug") == 0) ||
            config::EnvironmentConfig::GetBool(L"MM2HACK_DEBUG", false);

        if (config::EnvironmentConfig::GetBool(L"OUTPUT_LOG_ENABLE"))
        {
            using conf = config::SystemConfig;
            DxLib::SetApplicationLogSaveDirectory(conf::kLogFilePath.c_str());
            DxLib::SetApplicationLogFileName(conf::kDxLibLogFileName.c_str());
            DxLib::SetOutApplicationLogValidFlag(TRUE);
            utils::LogWriter::Initialize(conf::kLogFilePath);
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
            exceptions::ErrorHandler::Handle(
                L"Failed to initialize the window.",
                L"WindowManager",
                L"Initialize",
                exceptions::ErrorLevel::Error
            );
            return false;
        }

        if (DxLib::DxLib_Init() == -1)
        {
            exceptions::ErrorHandler::Handle(
                L"Failed to initialize DxLib.",
                L"WindowManager",
                L"Initialize",
                exceptions::ErrorLevel::Error
            );
            return false;
        }

        _mainWindowHandle = DxLib::GetMainWindowHandle();
        if (_mainWindowHandle == nullptr)
        {
            exceptions::ErrorHandler::Handle(
                L"Unable to obtain window handle.",
                L"WindowManager",
                L"Initialize",
                exceptions::ErrorLevel::Error
            );
            DxLib::DxLib_End();
            return false;
        }

        _dxLibWnd = reinterpret_cast<WNDPROC>(GetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC));
        SetWindowLongPtr(_mainWindowHandle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));

        if (DxLib::SetDrawScreen(DX_SCREEN_BACK) == -1)
        {
            exceptions::ErrorHandler::Handle(
                L"The SetDrawScreen(DX_SCREEN_BACK) function failed.",
                L"WindowManager",
                L"Initialize",
                exceptions::ErrorLevel::Error
            );
            DxLib::DxLib_End();
            return false;
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