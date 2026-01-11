//==============================================================================
// 
//  Project: mm2hack
//  SoundSettingsUI.h
// 
//  The layout of the sound settings UI for SettingsWindow.
// 
//==============================================================================
#pragma once

#include <string>
#include <Windows.h>

namespace mm2hack::core::ui
{
    // The layout of the sound settings UI
    class SoundSettingsUI
    {
    public:
        explicit SoundSettingsUI(HWND parent);
        ~SoundSettingsUI() {}

        // Create the controls for the sound settings UI
        void CreateControls();
        // Submit the selected settings.
        void ApplySettings() const;

        void OnScroll(WPARAM wParam, LPARAM lParam);
        void OnCommand(WPARAM wParam, LPARAM lParam) const;

    private:
        const std::wstring kClassName{ L"SoundSettingsUI" };

        HWND _parent;

        HWND _slider_master;
        HWND _slider_bgm;
        HWND _slider_se;
        HWND _check_sound_enabled;
        HWND _combo_source;

        mutable ULONGLONG _last_update_tick = 0;
        const ULONGLONG kUpdateInterval = 30;       // Update interval in milliseconds

        void createSlider_(LPCWSTR label, int x, int y, HWND& out_slider) const;    // Helper to create a slider control
        void addSoundSourceOptions_() const;                                        // Populate sound source options in the combo box
        void loadSettings_() const;                                                 // Load settings from config and apply to UI controls
    };
}