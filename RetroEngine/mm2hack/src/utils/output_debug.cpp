#include "pch.h"

#include "output_debug.h"

#include <debugapi.h>

namespace mm2hack::utils
{
    void debug_log(const std::wstring& message)
    {
        OutputDebugString((message + L"\n").c_str());
    }
}