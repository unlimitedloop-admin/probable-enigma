//==============================================================================
// 
//  Project: mm2hack
//  CommonUIStyle.h
// 
//  Management of Common UI style.
// 
//==============================================================================
#pragma once

#include <windows.h>

namespace mm2hack::core::ui
{
    // Settings for configurable UI elements
    class CommonUIStyle
    {
    public:
        CommonUIStyle();
        ~CommonUIStyle();

        HFONT GetUIFont() const;
        void ApplyUIFont(HWND hwnd) const;

    private:
        HFONT _font;
    };
}