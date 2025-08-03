//==============================================================================
// 
//  Project: mm2hack
//  SoundSettingsUI.h
// 
//  The layout of the sound settings UI for SettingsWindow.
// 
//==============================================================================
#pragma once

#include <windows.h>

namespace mm2hack::core::overlay
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
        HWND _parent;

        HWND _slider_master;
        HWND _slider_bgm;
        HWND _slider_se;
        HWND _check_sound_enabled;
        HWND _combo_source;

        mutable ULONGLONG _last_update_tick = 0;
        const ULONGLONG kUpdateInterval = 30;   // Update interval in milliseconds

        void CreateSlider(LPCWSTR label, int x, int y, HWND& out_slider) const;
        void AddSoundSourceOptions() const;
        void LoadSettings() const;
    };
}