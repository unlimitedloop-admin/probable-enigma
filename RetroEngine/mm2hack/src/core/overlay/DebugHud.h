//==============================================================================
// 
//  Project: mm2hack
//  DebugHud.h
// 
//  Display debug information on the screen.
// 
//==============================================================================
#pragma once

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

        void Draw() const;

    private:
        DebugHud() = default;
    };
}