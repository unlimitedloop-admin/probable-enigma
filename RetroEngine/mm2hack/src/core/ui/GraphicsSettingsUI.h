//==============================================================================
// 
//  Project: mm2hack
//  GraphicsSettingsUI.h
// 
//  The layout of the graphics settings UI for SettingWindow.
// 
//==============================================================================
#pragma once

#include <Windows.h>

namespace mm2hack::core::overlay
{
    // The layout of the graphics settings UI
    class GraphicsSettingsUI
    {
    public:
        explicit GraphicsSettingsUI(HWND parent);
        ~GraphicsSettingsUI() {}

        // Create the controls for the graphics settings UI
        void CreateControls();
        // Submit the selected settings.
        void ApplySettings() const;

    private:
        HWND _parent;
        HWND _combo_resolution;
        HWND _check_vsync;
        HWND _combo_framerate;

        void AddResolutionOptions() const;
        void AddFramerateOptions() const;
        void LoadSettings() const;
    };


    struct ResolutionOption
    {
        const wchar_t* label;
        float scale;
    };

    static constexpr ResolutionOption kResolutionOptions[] = {
        { L"256 x 240", 1.0f },
        { L"512 x 480", 2.0f },
        { L"1024 x 960", 4.0f },
    };
}