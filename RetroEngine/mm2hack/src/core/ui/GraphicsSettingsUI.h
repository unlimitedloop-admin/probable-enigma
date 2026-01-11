//==============================================================================
// 
//  Project: mm2hack
//  GraphicsSettingsUI.h
// 
//  The layout of the graphics settings UI for SettingWindow.
// 
//==============================================================================
#pragma once

#include <string>
#include <Windows.h>

namespace mm2hack::core::ui
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
        const std::wstring kClassName{ L"GraphicsSettingsUI" };

        HWND _parent;
        HWND _combo_resolution;
        HWND _check_vsync;
        HWND _combo_framerate;

        void addResolutionOptions_() const;             // Populate resolution options in the combo box
        void addFramerateOptions_() const;              // Populate framerate options in the combo box
        void loadSettings_();                           // Load settings from config and apply to UI controls

        int mapFpsToIndex_(int fps) noexcept;           // Map exact FPS to index in kFramerateOptions
        int mapFpsToNearestIndex_(int fps) noexcept;    // Map nearest FPS to index in kFramerateOptions
    };

    // Predefined resolution options
    struct ResolutionOption
    {
        const wchar_t* label;
        float scale;
    };

    // Resolution options available in the UI
    static constexpr ResolutionOption kResolutionOptions[] = {
        { L"256 x 240", 1.0f },
        { L"512 x 480", 2.0f },
        { L"1024 x 960", 4.0f },
    };

    // Predefined framerate options
    struct FramerateOption
    {
        const wchar_t* label;
        int targetFps;
    };

    // Framerate options available in the UI
    static constexpr FramerateOption kFramerateOptions[] = {
        { L"30 FPS", 30 },
        { L"60 FPS", 60 },
        { L"120 FPS", 120 },
        { L"144 FPS", 144 },
        { L"240 FPS", 240 },
        { L"not restricted", 0 },  // 0 means no limit
    };
}