#include "pch.h"

#include "SymbolicCodeParser.h"

#include <cstdlib>
#include <regex>
#include "ICodeParser.h"

namespace mm2hack::core::cheats
{
    bool SymbolicCodeParser::TryParse(const std::wstring& code, std::vector<CheatPatch>& out_patches)
    {
        std::wregex rx(LR"(^\s*([A-Za-z0-9_\.]+)\s*=\s*([0-9A-Fa-fx]+)(?:\s+(freeze))?\s*$)");
        std::wsmatch m;
        if (!std::regex_match(code, m, rx))
        {
            return false;
        }

        CheatPatch p{};
        p.symbolic_path = m[1].str();

        const std::wstring v = m[2].str(); // allow "0x.." or decimal
        p.value = (v.rfind(L"0x", 0) == 0 || v.rfind(L"0X", 0) == 0)
            ? std::wcstoull(v.c_str(), nullptr, 16)
            : std::wcstoull(v.c_str(), nullptr, 10);

        p.freeze = m[3].matched;
        p.label = *p.symbolic_path;

        out_patches.push_back(std::move(p));
        return true;
    }
}