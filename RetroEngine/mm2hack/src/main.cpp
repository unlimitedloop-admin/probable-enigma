//==============================================================================
//
//  mm2hack - Game file that imitates Mega Man II.
//
//  Copyright (c) 2025 UNLIMITED LOOP ROOT-ONE
//  All rights reserved.
//
//  This software(and source code) is completely Unlicense.
//  See 'LICENSE'.
//
//==============================================================================
#include "pch.h"

#include <cstdlib>
#include <sal.h>
#include "core/bootstrap.h"

/// Entry point
/// ENTRYPOINT
/// main()
/// PG_START
INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    using namespace mm2hack::core;

    // Initialize the application environment.
    Bootstrapper(lpCmdLine);

    // Run the WindowManager.
    RunWindowManager(hInstance, lpCmdLine, nCmdShow);

    // Clean up resources before exiting.
    CleanUp(lpCmdLine);

    return EXIT_SUCCESS;
}
