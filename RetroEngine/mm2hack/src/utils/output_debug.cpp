#include "output_debug.h"

#include <string>
#include <Windows.h>

namespace mm2hack::utils
{
    void debug_log(const std::wstring& message)
    {
        OutputDebugString((message + L"\n").c_str());
    }
}