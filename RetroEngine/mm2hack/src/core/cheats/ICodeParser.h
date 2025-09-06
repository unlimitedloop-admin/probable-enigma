//==============================================================================
// 
//  Project: mm2hack
//  ICodeParser.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace mm2hack::core::cheats
{
    struct CheatPatch
    {
        // If symbolic_path is set, use it; otherwise use address.
        std::optional<std::wstring> symbolic_path;
        std::optional<uint32_t>     address;

        uint64_t value{ 0 };                // up to 64-bit for future-proof
        std::optional<uint64_t> compare;    // optional compare
        bool freeze{ false };               // whether to freeze or just write once
        std::wstring label;                 // for UI
    };

    class ICodeParser
    {
    public:
        virtual ~ICodeParser() = default;
        virtual bool TryParse(const std::wstring& code, std::vector<CheatPatch>& out_patches) = 0;
    };
}
