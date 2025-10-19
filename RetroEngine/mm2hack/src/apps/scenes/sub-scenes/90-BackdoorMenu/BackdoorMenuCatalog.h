#pragma once

#include <array>
#include <optional>
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

        enum class Action : unsigned char
        {
            None = 0,
            Back,
            Top,
            Enter,
            NextScene,
        };

        struct InsideMenuItemDesc
        {
            std::wstring_view label;    // Display label
            bool selectable;            // Available for selection
            Action action;              // Action to perform on activation
            int advanceLines;           // Next entry is this many lines below (for spacing)
            std::optional<int> subId;   // Sub-menu ID to enter (if applicable)
            //TODO: [InsideMenuItemDesc] std::optional<int> value;   // Parameter value (if applicable)
        };

        inline constexpr std::array<InsideMenuItemDesc, 2> kInsideMenu_CompleteArsenal
        {
            InsideMenuItemDesc{ L"ALL WEAPONS UNLOCKED.", false, Action::None, 2, std::nullopt },
            InsideMenuItemDesc{ L"BACK",                  true,  Action::Back, 1, std::nullopt },
        };

        inline constexpr std::array<InsideMenuItemDesc, 3> kInsideMenu_Stages
        {
            InsideMenuItemDesc{ L"DEMO STAGE 1", true, Action::Enter, 1, 0 },
            InsideMenuItemDesc{ L"DEMO STAGE 2", true, Action::Enter, 2, 0 },
            InsideMenuItemDesc{ L"BACK",         true, Action::Back,  1, std::nullopt },
        };

        inline constexpr std::array<InsideMenuItemDesc, 4> kInsideMenu_StageEdit
        {
            InsideMenuItemDesc{ L"ROOM NO. 00", true,  Action::None,      1, std::nullopt },
            InsideMenuItemDesc{ L"BEGIN",       true,  Action::NextScene, 1, std::nullopt },
            InsideMenuItemDesc{ L"BACK",        true,  Action::Back,      1, std::nullopt },
            InsideMenuItemDesc{ L"GO TO TOP",   true,  Action::Top,       1, std::nullopt },
        };

        inline constexpr std::array<InsideMenuItemDesc, 3> kInsideMenu_ResetParameter
        {
            InsideMenuItemDesc{ L"RESET ALL PARAMETERS", false, Action::None, 1, std::nullopt },
            InsideMenuItemDesc{ L"TO DEFAULT VALUES.",   false, Action::None, 2, std::nullopt },
            InsideMenuItemDesc{ L"BACK",                 true,  Action::Back, 1, std::nullopt },
        };


        // ========= Jump to Any Scene Parameter =========

    }
}