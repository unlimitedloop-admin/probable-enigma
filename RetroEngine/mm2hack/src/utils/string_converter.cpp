#include "string_converter.h"

#include <string>
#include <Windows.h>

namespace mm2hack::utils
{
    // Convert a UTF-8 string to a wide string (UTF-16)
    std::wstring utf8_to_wstring(const std::string& utf8Str)
    {
        if (utf8Str.empty()) return L"";

        const int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
        if (sizeNeeded == 0) return L"(conversion failed)";

        std::wstring result(sizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &result[0], sizeNeeded);

        // Delete the null terminator
        result.resize(wcslen(result.c_str()));
        return result;
    }

    // Overloaded function to convert a UTF-8 C-style string to a wide string
    std::wstring utf8_to_wstring(const char* utf8Str)
    {
        return utf8Str ? utf8_to_wstring(std::string(utf8Str)) : L"";
    }

    // Convert a wide string (UTF-16) to a UTF-8 string
    std::string wstring_to_utf8(const std::wstring& wide)
    {
        if (wide.empty()) return {};

        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8Len == 0) return {};

        std::string utf8(utf8Len - 1, 0); // -1 to exclude null terminator
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), utf8Len, nullptr, nullptr);

        return utf8;
    }
}