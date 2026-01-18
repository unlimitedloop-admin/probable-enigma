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
#include <Windows.h>
#include "CheatCodeInjectionUI.h"
#include "CommonUIStyle.h"
#include "GraphicsSettingsUI.h"
#include "SoundSettingsUI.h"

namespace mm2hack::core::ui
{
    // Child window for settings, allowing users to adjust graphics and sound settings
    class SettingsWindow
    {
    public:
        enum class Tab
        {
            Graphics,
            Sound,
            Cheats
        };

        // Layout properties for each settings tab
        static constexpr SettingTabLayout kWindowProps[] = {
            { L"Graphics", 350, 320,  80, 240, 170, 240, 240, 240 },
            { L"Sound",    400, 380, 130, 300, 220, 300, 290, 300 },
            { L"Cheats",   860, 600, 590, 520, 680, 520, 750, 520 }
        };

        static void RegisterWindowClass(HINSTANCE hInstance);
        static void OpenTab(HWND parent, Tab tab);

    private:
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        void applySettings_();
        void applySoundToAudio_();
        void setHandle_(HWND hwnd);
        void createContent_(Tab tab);
        static SettingsWindow* getThis_(HWND hwnd);

    private:
        inline static constexpr LPCWSTR kClassName{ L"SettingsWindowClass" };

        static HWND _hwnd;
        static Tab _current_tab;
        std::unique_ptr<CheatCodeInjectionUI> _cheat_ui;
        std::unique_ptr<GraphicsSettingsUI> _graphics_ui;
        std::unique_ptr<SoundSettingsUI> _sound_ui;
        bool _is_closing{ false };

        static std::unique_ptr<SettingsWindow> _instance;   // Just to own the instance
    };
}