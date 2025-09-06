//==============================================================================
// 
//  Project: mm2hack
//  SymbolicCodeParser.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <string>
#include <vector>
#include "ICodeParser.h"

namespace mm2hack::core::cheats
{
    class SymbolicCodeParser final : public ICodeParser
    {
    public:
        SymbolicCodeParser() = default;
        ~SymbolicCodeParser() override = default;

        bool TryParse(const std::wstring& code, std::vector<CheatPatch>& out_patches) override;
    };
}
