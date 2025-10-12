#pragma once

#include <array>
#include <string_view>

namespace mm2hack::apps::scenes
{
    namespace BackdoorMenu_
    {
        inline constexpr std::array<std::wstring_view, 8> kTopMenuTitles{
            L"COMPLETE ARSENAL       ",
            L"PARAMETER CONFIGURATION",
            L"VIEWER MODE            ",
            L"STAGES                 ",
            L"REGULAR BOOT           ",
            L"SOUND TEST MODE        ",
            L"SPRITE TEST            ",
            L"RESET PARAMETER        "
        };
    }
}