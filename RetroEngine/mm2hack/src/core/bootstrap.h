//==============================================================================
// 
//  Project: mm2hack
//  bootstrap.h
// 
//  Set up the application environment and run the main process.
// 
//==============================================================================
#pragma once

#include <Windows.h>

namespace mm2hack::core
{
    void Bootstrapper(LPWSTR lpCmdLine);
    void RunWindowManager(HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow);
    void CleanUp();
}