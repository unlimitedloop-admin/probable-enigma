//==============================================================================
// 
//  Project: mm2hack
//  output_debug.h
// 
//  Output debug utilities.
// 
//==============================================================================
#pragma once

#include <format>
#include <string>
#include <Windows.h>

namespace mm2hack::utils
{
    // Prints a character to the output console.
    void debug_log(const std::wstring& message);
    // Prints a character to the output console (with string formatting).
    template <typename... Args>
    void debug_log(const std::wstring& formatStr, Args&&... args)
    {
        std::wstring message = std::vformat(formatStr, std::make_wformat_args(std::forward<Args>(args)...));
        OutputDebugString((message + L"\n").c_str());
    }
}