//==============================================================================
// 
//  Project: mm2hack
//  DebugHud.h
// 
//  Display debug information on the screen.
// 
//==============================================================================
#pragma once

#include <string>

namespace mm2hack::core::overlay
{
    // Debug HUD for displaying debug information on the screen
    class DebugHud
    {
    public:
        static DebugHud& GetInstance()
        {
            static DebugHud instance;
            return instance;
        }

        // Draw the debug HUD 
        void Draw() const;

    private:
        DebugHud() = default;

    private:
        const std::wstring kClassName = L"DebugHud";
    };
}