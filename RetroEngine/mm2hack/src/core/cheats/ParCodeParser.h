//==============================================================================
// 
//  Project: mm2hack
//  ParCodeParser.h
// 
//  Cheat code parser for the mm2hack project.
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include "ICodeParser.h"

namespace mm2hack::core::cheats
{
    // A parser for the PAR-style cheat code format
    class ParCodeParser final : public ICodeParser
    {
    public:
        ParCodeParser() = default;
        ~ParCodeParser() override = default;

        // Attempt to parse the given PAR-style code string into one or more CheatPatch entries
        bool TryParse(const std::wstring& code, std::vector<CheatPatch>& out_patches) override;
    };
}