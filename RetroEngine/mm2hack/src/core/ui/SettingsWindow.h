//==============================================================================
// 
//  Project: mm2hack
//  SettingsWindow.h
// 
//  Setting window for adjusting graphics and sound settings for the application.
// 
//==============================================================================
#pragma once

#include <memory>
#include <windows.h>
#include "GraphicsSettingsUI.h"
#include "SoundSettingsUI.h"

namespace mm2hack::core::overlay
{
    // Child window for settings, allowing users to adjust graphics and sound settings
    class SettingsWindow
    {
    public:
        enum class Tab
        {
            Graphics,
            Sound
        };

        static void RegisterWindowClass(HINSTANCE hInstance);
        static void OpenTab(HWND parent, Tab tab);

    private:
        static constexpr LPCWSTR kClassName = L"SettingsWindowClass";
        static HWND _hwnd;
        static Tab _current_tab;
        std::unique_ptr<GraphicsSettingsUI> _graphics_ui;
        std::unique_ptr<SoundSettingsUI> _sound_ui;

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        void ApplySettings();
        void ApplySoundToAudio();
        void SetHandle(HWND hwnd);
        void CreateContent(Tab tab);
        static SettingsWindow* GetThis(HWND hwnd);

        static std::unique_ptr<SettingsWindow> _instance;   // Just to own the instance
    };
}