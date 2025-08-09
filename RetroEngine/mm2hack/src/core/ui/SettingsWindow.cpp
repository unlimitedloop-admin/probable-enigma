#include "pch.h"

#include "SettingsWindow.h"

#include <libloaderapi.h>
#include "apps/deal/GameContext.h"
#include "config/ConfigUIManager.h"
#include "config/SoundConfig.h"
#include "core/ui/CommonUIStyle.h"
#include "GraphicsSettingsUI.h"
#include "SoundSettingsUI.h"

namespace mm2hack::core::overlay
{
    constexpr int ID_BUTTON_OK = 9001;
    constexpr int ID_BUTTON_CANCEL = 9002;
    constexpr int ID_BUTTON_APPLY = 9003;

    HWND SettingsWindow::_hwnd = nullptr;
    SettingsWindow::Tab SettingsWindow::_current_tab = Tab::Graphics;
    std::unique_ptr<SettingsWindow> SettingsWindow::_instance = nullptr;

    void SettingsWindow::RegisterWindowClass(HINSTANCE hInstance)
    {
        WNDCLASS wc{};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = kClassName;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);

        RegisterClass(&wc);
    }

    void SettingsWindow::OpenTab(HWND parent, Tab tab)
    {
        _current_tab = tab;

        if (_hwnd != nullptr)
        {
            SetForegroundWindow(_hwnd);
            return;
        }

        _instance = std::make_unique<SettingsWindow>();

        _hwnd = CreateWindowEx(
            WS_EX_DLGMODALFRAME,
            kClassName,
            L"Settings",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            400, 350,
            parent,
            nullptr,
            GetModuleHandle(nullptr),
            _instance.get()
        );
    }

    LRESULT CALLBACK SettingsWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        SettingsWindow* self = nullptr;

        if (msg == WM_NCCREATE)
        {
            auto createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
            self = reinterpret_cast<SettingsWindow*>(createStruct->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        else
        {
            self = reinterpret_cast<SettingsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (self)
        {
            switch (msg)
            {
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;
            case WM_COMMAND:
            {
                const int id = LOWORD(wParam);
                switch (id)
                {
                case ID_BUTTON_OK:
                    self->ApplySettings();
                    DestroyWindow(hwnd);
                    return 0;
                case ID_BUTTON_CANCEL:
                    DestroyWindow(hwnd);
                    return 0;
                case ID_BUTTON_APPLY:
                    self->ApplySettings();
                    return 0;
                }
                break;
            }
            case WM_CREATE:
                self->SetHandle(hwnd);
                self->CreateContent(_current_tab);
                return 0;
            case WM_DESTROY:
                _hwnd = nullptr;
                _instance.reset();
                return 0;
            }
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    void SettingsWindow::ApplySettings()
    {
        if (_graphics_ui)
        {
            _graphics_ui->ApplySettings();
        }
        if (_sound_ui)
        {
            _sound_ui->ApplySettings();
            ApplySoundToAudio();
        }
    }

    void SettingsWindow::ApplySoundToAudio()
    {
        config::SoundConfig cfg;
        config::ConfigUIManager::LoadSoundConfig(cfg);

        auto& ctx = apps::deal::GameContext::GetInstance();
        if (ctx.IsInitialized())
        {
            auto& audio = ctx.GetResourceManager().GetAudioManager();
            audio.SetMasterVolume(cfg.master);
            audio.SetBgmVolume(cfg.bgm);
            audio.SetSeVolume(cfg.se);
            audio.SetEnabled(cfg.enabled);
        }
    }

    void SettingsWindow::SetHandle(HWND hwnd)
    {
        _hwnd = hwnd;
    }

    void SettingsWindow::CreateContent(Tab tab)
    {
        ui::CommonUIStyle uiStyle;

        if (tab == Tab::Graphics)
        {
            _graphics_ui = std::make_unique<GraphicsSettingsUI>(_hwnd);
            _graphics_ui->CreateControls();
        }
        if (tab == Tab::Sound)
        {
            _sound_ui = std::make_unique<SoundSettingsUI>(_hwnd);
            _sound_ui->CreateControls();
        }

        HWND buttonOK = CreateWindowEx(0, L"BUTTON", L"OK",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            220, 280, 60, 28,
            _hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_BUTTON_OK)), GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(buttonOK);

        HWND buttonCancel = CreateWindowEx(0, L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            290, 280, 80, 28,
            _hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_BUTTON_CANCEL)), GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(buttonCancel);

        HWND buttonApply = CreateWindowEx(0, L"BUTTON", L"Apply",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            130, 280, 80, 28,
            _hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_BUTTON_APPLY)), GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(buttonApply);
    }

    SettingsWindow* SettingsWindow::GetThis(HWND hwnd)
    {
        return hwnd ? reinterpret_cast<SettingsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) : nullptr;
    }
}