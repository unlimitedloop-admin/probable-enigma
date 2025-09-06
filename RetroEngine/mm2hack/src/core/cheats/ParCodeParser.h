//==============================================================================
// 
//  Project: mm2hack
//  ParCodeParser.h
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
    class ParCodeParser final : public ICodeParser
    {
    public:
        ParCodeParser() = default;
        ~ParCodeParser() override = default;

        bool TryParse(const std::wstring& code, std::vector<CheatPatch>& out_patches) override;
    };
}
