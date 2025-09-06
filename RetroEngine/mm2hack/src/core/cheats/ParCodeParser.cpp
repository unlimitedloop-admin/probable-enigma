#include "pch.h"

#include "ParCodeParser.h"

#include <cstdlib>
#include <regex>
#include "ICodeParser.h"

namespace mm2hack::core::cheats
{
    // Accept:  L"PAR 00A123:FF freeze"  or  L"PAR 00A123:FF C=02"
    bool ParCodeParser::TryParse(const std::wstring& code, std::vector<CheatPatch>& out_patches)
    {
        std::wregex rx(LR"(^\s*PAR\s+([0-9A-Fa-f]{4,8})\s*:\s*([0-9A-Fa-f]{1,16})(?:\s+C\s*=\s*([0-9A-Fa-f]{1,16}))?(?:\s+(freeze))?\s*$)");
        std::wsmatch m;
        if (!std::regex_match(code, m, rx))
        {
            return false;
        }

        CheatPatch p{};
        p.address = std::wcstoul(m[1].str().c_str(), nullptr, 16);
        p.value = std::wcstoull(m[2].str().c_str(), nullptr, 16);
        if (m[3].matched)
        {
            p.compare = std::wcstoull(m[3].str().c_str(), nullptr, 16);
        }
        p.freeze = m[4].matched;
        p.label = L"PAR " + m[1].str();

        out_patches.push_back(std::move(p));
        return true;
    }
}
