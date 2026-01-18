//==============================================================================
// 
//  Project: mm2hack
//  decimal_decoder.h
// 
//  Decimal decoder utility functions.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::utils
{
    // Decode a floating-point number into its hexadecimal string representation
    std::wstring decode_floating_hex_number(double expression);
}