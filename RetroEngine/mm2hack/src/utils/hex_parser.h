//==============================================================================
//
//  Project: mm2hack
//  hex_parser.h
//
//  Parses the "0x"-prefixed hex strings the data files use for gameplay
//  integers (HP, attack power, NES palette indices, ...).
//
//==============================================================================
#pragma once

#include <cstdint>
#include <string_view>

namespace mm2hack::utils
{
    // Parses "0x0A" / "0X0a" style text (1-8 hex digits after the prefix).
    // Returns false for anything else, including a missing prefix or digits.
    [[nodiscard]] inline bool try_parse_hex(std::string_view text, std::int64_t& out) noexcept
    {
        if (text.size() < 3 || text.size() > 10 || text[0] != '0' || (text[1] != 'x' && text[1] != 'X'))
        {
            return false;
        }

        std::int64_t value = 0;
        for (std::size_t i = 2; i < text.size(); ++i)
        {
            const char c = text[i];
            int digit = 0;
            if (c >= '0' && c <= '9') digit = c - '0';
            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
            else return false;
            value = value * 16 + digit;
        }
        out = value;
        return true;
    }
}
