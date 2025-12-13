//==============================================================================
// 
//  Project: mm2hack
//  string_converter.h
// 
//  String converting utilities.
// 
//==============================================================================
#pragma once

#include <initializer_list>
#include <string>
#include <string_view>

namespace mm2hack::utils
{
    // Convert a UTF-8 string to a wide string (UTF-16)
    std::wstring utf8_to_wstring(const std::string& utf8Str);
    // Overloaded function to convert a UTF-8 C-style string to a wide string
    std::wstring utf8_to_wstring(const char* utf8Str);
    // Convert a wide string (UTF-16) to a UTF-8 string
    std::string wstring_to_utf8(const std::wstring& wide);
    // Concatenate multiple wide string parts into a provided buffer
    wchar_t* concat_to_wchar_buffer(wchar_t* outBuf, std::size_t outBufSize, std::initializer_list<std::wstring_view> parts) noexcept;
}