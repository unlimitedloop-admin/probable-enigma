#include "pch.h"

#include "bootstrap.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>
#include <d3d9.h>
#include <d3d9caps.h>
#include <d3d9types.h>
#include <sysinfoapi.h>
#include <VersionHelpers.h>
#include "config/EnvironmentConfig.h"
#include "winapi/WindowManager.h"

namespace mm2hack::core
{
    void Bootstrapper(LPWSTR lpCmdLine)
    {
        // Load the environment configurations
        config::EnvironmentConfig::LoadFromFile(L"mm2hack.env");

        // Check if the application is in debug mode (via command-line or environment variable)
        const bool isDebugMode =
            (lstrcmp(lpCmdLine, L"debug") == 0) ||
            config::EnvironmentConfig::GetBool(L"MM2HACK_DEBUG", false);
        bool passSystemCheck = true;

        // Perform system checks (e.g., OS version, memory, Direct3D capabilities)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_UNKNOWN)
        {
            if (isDebugMode)
            {
                exceptions::ErrorHandler::Handle(
                    L"Unsupported processor architecture.",
                    L"-",
                    L"Bootstrapper",
                    exceptions::ErrorLevel::Info
                );
            }
            passSystemCheck = false;
        }

        if (!IsWindows10OrGreater())
        {
            if (isDebugMode)
            {
                exceptions::ErrorHandler::Handle(
                    L"Unsupported Windows version(requires Windows 10 or later).",
                    L"-",
                    L"Bootstrapper",
                    exceptions::ErrorLevel::Info
                );
            }
            passSystemCheck = false;
        }

        MEMORYSTATUSEX memStatus{};
        memStatus.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memStatus);
        constexpr ULONGLONG kMinPhysMemory = 512ULL * 1024 * 1024;
        if (memStatus.ullTotalPhys < kMinPhysMemory)
        {
            if (isDebugMode)
            {
                exceptions::ErrorHandler::Handle(
                    L"Insufficient physical memory(< 512MB).",
                    L"-",
                    L"Bootstrapper",
                    exceptions::ErrorLevel::Warning
                );
            }
            passSystemCheck = false;
        }

        IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D)
        {
            if (isDebugMode)
            {
                exceptions::ErrorHandler::Handle(
                    L"Direct3D 9 initialization failed (DirectX 9 not available)",
                    L"-",
                    L"Bootstrapper",
                    exceptions::ErrorLevel::FatalError
                );
            }
        }

        D3DCAPS9 d3dCaps;
        if (FAILED(pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps)))
        {
            pD3D->Release();
            if (isDebugMode)
            {
                exceptions::ErrorHandler::Handle(
                    L"Failed to retrieve Direct3D device capabilities",
                    L"-",
                    L"Bootstrapper",
                    exceptions::ErrorLevel::FatalError
                );
            }
        }
        if (d3dCaps.VertexShaderVersion < D3DVS_VERSION(2, 0) || d3dCaps.PixelShaderVersion < D3DPS_VERSION(2, 0))
        {
            pD3D->Release();
            if (isDebugMode)
            {
                exceptions::ErrorHandler::Handle(
                    L"Shader Model < 2.0 is not supported",
                    L"-",
                    L"Bootstrapper",
                    exceptions::ErrorLevel::FatalError
                );
            }
        }

        pD3D->Release();

        if (isDebugMode)
        {
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
            _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
            _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
            _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);

            if (passSystemCheck)
            {
                MessageBox(nullptr,
                    L"System check passed. Your system meets the minimum requirements for running mm2hack.",
                    L"System Check", MB_OK | MB_ICONINFORMATION);
            }
            else
            {
                MessageBox(nullptr,
                    L"System check failed. Please see the log for details.",
                    L"System Check", MB_OK | MB_ICONERROR);
            }
        }
    }

    void RunWindowManager(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow)
    {
        std::wstring windowTitle = config::EnvironmentConfig::Get(L"WINDOW_TEXT", L"mm2hack.demo") + L" " +
            config::EnvironmentConfig::Get(L"MM2HACK_VERSION");

        winapi::WindowManager& windowManager = winapi::WindowManager::GetInstance();
        if (!windowManager.Initialize(hInstance, lpCmdLine, nCmdShow, windowTitle))
        {
            MessageBox(nullptr, L"Failed to initialize window manager.", L"Error", MB_OK | MB_ICONERROR);
            exit(EXIT_FAILURE);
        }
        windowManager.RunMainLoop();
        windowManager.Shutdown();
    }

    void CleanUp(LPWSTR lpCmdLine)
    {
    }
}