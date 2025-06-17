#include "bootstrap.h"

#include <crtdbg.h>
#include <cstdlib>
#include <d3d9.h>
#include <VersionHelpers.h>
#include <Windows.h>
#include "winapi/WindowManager.h"

namespace mm2hack::core
{
    void Bootstrapper(LPWSTR lpCmdLine)
    {
        bool isDebugMode = !lstrcmp(lpCmdLine, L"debug");
        bool passSystemCheck = true;

        // Load the environment configurations
        //config::EnvironmentConfig::LoadFromFile(L"mm2hack.env");

        // Perform system checks (e.g., OS version, memory, Direct3D capabilities)
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_UNKNOWN)
        {
            passSystemCheck = false;
        }

        if (!IsWindows10OrGreater())
        {
            passSystemCheck = false;
        }

        MEMORYSTATUSEX memStatus{};
        memStatus.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memStatus);
        if (memStatus.ullTotalPhys < static_cast<unsigned long long>(512 * 1024) * 1024)
        {
            passSystemCheck = false;
        }

        IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (!pD3D)
        {
            exit(EXIT_FAILURE);
        }

        D3DCAPS9 d3dCaps;
        if (FAILED(pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dCaps)))
        {
            pD3D->Release();
            exit(EXIT_FAILURE);
        }
        if (d3dCaps.VertexShaderVersion < D3DVS_VERSION(2, 0) || d3dCaps.PixelShaderVersion < D3DPS_VERSION(2, 0))
        {
            pD3D->Release();
            exit(EXIT_FAILURE);
        }

        pD3D->Release();

        if (isDebugMode)
        {
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

            if (passSystemCheck)
            {
                MessageBoxW(nullptr,
                    L"System check passed. Your system meets the minimum requirements for running mm2hack.",
                    L"System Check", MB_OK | MB_ICONINFORMATION);
            }
            else
            {
                MessageBoxW(nullptr,
                    L"System check failed. Please see the log for details.",
                    L"System Check", MB_OK | MB_ICONERROR);
            }
        }
    }

    void RunMainProcess(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow)
    {
        winapi::WindowManager& windowManager = winapi::WindowManager::GetInstance();
        if (!windowManager.Initialize(hInstance, lpCmdLine, nCmdShow, L"mm2hack.demo"))
        {
            MessageBoxW(nullptr, L"Failed to initialize window manager.", L"Error", MB_OK | MB_ICONERROR);
            exit(EXIT_FAILURE);
        }
        windowManager.RunMainLoop();
        windowManager.Shutdown();
    }

    void CleanUp()
    {

    }
}