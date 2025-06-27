//==============================================================================
// 
//  Project: mm2hack
//  string_converter.h
// 
//  String converting utilities.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::utils
{
    // Convert a UTF-8 string to a wide string (UTF-16)
    std::wstring utf8_to_wstring(const std::string& utf8Str);
    // Overloaded function to convert a UTF-8 C-style string to a wide string
    std::wstring utf8_to_wstring(const char* utf8Str);
    // Convert a wide string (UTF-16) to a UTF-8 string
    std::string wstring_to_utf8(const std::wstring& wide);
}