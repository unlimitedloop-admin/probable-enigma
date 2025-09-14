//==============================================================================
// 
//  Project: mm2hack
//  SymbolicCodeParser.h
// 
//  Cheat code parser for symbolic cheat codes.
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include "ICodeParser.h"

namespace mm2hack::core::cheats
{
    // A parser for symbolic cheat code format
    class SymbolicCodeParser final : public ICodeParser
    {
    public:
        SymbolicCodeParser() = default;
        ~SymbolicCodeParser() override = default;

        // Attempt to parse the given symbolic code string into one or more CheatPatch entries
        bool TryParse(const std::wstring& code, std::vector<CheatPatch>& out_patches) override;
    };
}