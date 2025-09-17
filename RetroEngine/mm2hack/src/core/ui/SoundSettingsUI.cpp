#include "pch.h"

#include "SoundSettingsUI.h"

#include <CommCtrl.h>
#include <sysinfoapi.h>
#include <Windowsx.h>
#include "apps/deal/GameContext.h"
#include "CommonUIStyle.h"
#include "config/ConfigUIManager.h"
#include "config/SoundConfig.h"

namespace mm2hack::core::ui
{
    SoundSettingsUI::SoundSettingsUI(HWND parent)
        : _parent(parent),
        _slider_master(nullptr), _slider_bgm(nullptr), _slider_se(nullptr),
        _check_sound_enabled(nullptr), _combo_source(nullptr)
    {
    }

    void SoundSettingsUI::CreateControls()
    {
        ui::CommonUIStyle uiStyle;

        // Master Volume
        CreateSlider(L"Master Vol:", 20, 20, _slider_master);
        // BGM
        CreateSlider(L"BGM Vol:", 20, 70, _slider_bgm);
        // SE
        CreateSlider(L"SE Vol:", 20, 120, _slider_se);

        // Sound Enabled Checkbox
        _check_sound_enabled = CreateWindowEx(0, L"BUTTON", L"Sound Output?",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 170, 200, 20,
            _parent, nullptr, nullptr, nullptr);
        uiStyle.ApplyUIFont(_check_sound_enabled);

        // Sound Source ComboBox
        auto label = CreateWindowEx(0, L"STATIC", L"Sound Engine:",
            WS_CHILD | WS_VISIBLE,
            20, 210, 100, 20,
            _parent, nullptr, nullptr, nullptr);
        uiStyle.ApplyUIFont(label);

        _combo_source = CreateWindowEx(0, L"COMBOBOX", nullptr,
            CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
            130, 210, 150, 100,
            _parent, nullptr, nullptr, nullptr);
        uiStyle.ApplyUIFont(_combo_source);
        AddSoundSourceOptions();

        LoadSettings();
    }

    void SoundSettingsUI::ApplySettings() const
    {
        config::SoundConfig config;
        config.master = static_cast<int>(SendMessage(_slider_master, TBM_GETPOS, 0, 0));
        config.bgm = static_cast<int>(SendMessage(_slider_bgm, TBM_GETPOS, 0, 0));
        config.se = static_cast<int>(SendMessage(_slider_se, TBM_GETPOS, 0, 0));
        config.enabled = (Button_GetCheck(_check_sound_enabled) == BST_CHECKED);
        config.sourceIndex = static_cast<int>(SendMessage(_combo_source, CB_GETCURSEL, 0, 0));

        // Save to INI.
        config::ConfigUIManager::SaveSoundConfig(config);

        // To apply the settings, we need to access the ResourceManager.
        auto& ctx = apps::deal::GameContext::GetInstance();
        if (ctx.GetResourceManagerPtr())
        {
            auto& audio = ctx.GetResourceManager().GetAudioManager();
            audio.SetMasterVolume(config.master);
            audio.SetBgmVolume(config.bgm);
            audio.SetSeVolume(config.se);
            audio.SetEnabled(config.enabled);
        }
    }

    void SoundSettingsUI::OnScroll(WPARAM wParam, LPARAM lParam)
    {
        ULONGLONG now = GetTickCount64();
        if (now - _last_update_tick < kUpdateInterval)
            return;     // Throttle updates

        _last_update_tick = now;
        HWND hwnd = (HWND)lParam;
        if (hwnd == _slider_master || hwnd == _slider_bgm || hwnd == _slider_se)
        {
            ApplySettings();
        }
    }

    void SoundSettingsUI::OnCommand(WPARAM wParam, LPARAM lParam) const
    {
        HWND hwnd = (HWND)lParam;
        if (hwnd == _check_sound_enabled || hwnd == _combo_source)
        {
            ApplySettings();
        }
    }

    void SoundSettingsUI::CreateSlider(LPCWSTR label, int x, int y, HWND& out_slider) const
    {
        CreateWindowEx(0, L"STATIC", label,
            WS_CHILD | WS_VISIBLE,
            x, y, 120, 20,
            _parent, nullptr, nullptr, nullptr);

        out_slider = CreateWindowEx(0, TRACKBAR_CLASS, nullptr,
            WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            x + 130, y, 150, 30,
            _parent, nullptr, nullptr, nullptr);

        SendMessage(out_slider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendMessage(out_slider, TBM_SETPOS, TRUE, 80);
    }

    void SoundSettingsUI::AddSoundSourceOptions() const
    {
        SendMessage(_combo_source, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Original Style"));
        SendMessage(_combo_source, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"+DPCM"));
        SendMessage(_combo_source, CB_SETCURSEL, 0, 0);
    }

    void SoundSettingsUI::LoadSettings() const
    {
        config::SoundConfig config;
        config::ConfigUIManager::LoadSoundConfig(config);

        SendMessage(_slider_master, TBM_SETPOS, TRUE, config.master);
        SendMessage(_slider_bgm, TBM_SETPOS, TRUE, config.bgm);
        SendMessage(_slider_se, TBM_SETPOS, TRUE, config.se);
        Button_SetCheck(_check_sound_enabled, config.enabled ? BST_CHECKED : BST_UNCHECKED);
        SendMessage(_combo_source, CB_SETCURSEL, config.sourceIndex, 0);
    }
}