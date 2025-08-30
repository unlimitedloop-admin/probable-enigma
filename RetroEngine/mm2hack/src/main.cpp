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

INT APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    using namespace mm2hack;

    core::Bootstrapper(lpCmdLine);
    core::RunMainProcess(hInstance, lpCmdLine, nCmdShow);
    core::CleanUp();

    return EXIT_SUCCESS;
}
