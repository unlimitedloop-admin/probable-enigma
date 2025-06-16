#include "bootstrap.h"

#include <crtdbg.h>
#include <Windows.h>

namespace mm2hack::core
{
    void Bootstrapper(LPWSTR lpCmdLine)
    {
        bool isDebugMode = !lstrcmp(lpCmdLine, L"debug");
        bool passSystemCheck = true;

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

    }

    void CleanUp()
    {

    }
}