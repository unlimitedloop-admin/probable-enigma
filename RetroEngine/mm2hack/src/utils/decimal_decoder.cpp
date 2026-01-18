#include "pch.h"

#include "decimal_decoder.h"

#include <cstdio>

namespace mm2hack::utils
{
    std::wstring decode_floating_hex_number(double expression)
    {
        int int_part = (int)expression;
        double frac_part = (expression - int_part) * 0x100;
        wchar_t buffer[20];
        swprintf(buffer, 20, L"%02X.%02X", int_part, static_cast<unsigned int>(frac_part));
        return std::wstring(buffer);
    }
}