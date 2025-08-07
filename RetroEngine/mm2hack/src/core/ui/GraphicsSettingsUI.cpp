#include "pch.h"

#include "GraphicsSettingsUI.h"

#include <iterator>
#include <libloaderapi.h>
#include <Windowsx.h>
#include "CommonUIStyle.h"
#include "config/ConfigUIManager.h"
#include "core/winapi/WindowManager.h"
#include "utils/FpsManager.h"

namespace mm2hack::core::overlay
{
    GraphicsSettingsUI::GraphicsSettingsUI(HWND parent)
        : _parent(parent),
        _combo_resolution(nullptr),
        _check_vsync(nullptr),
        _combo_framerate(nullptr)
    {
    }

    void GraphicsSettingsUI::CreateControls()
    {
        ui::CommonUIStyle uiStyle;

        // Resolution ComboBox
        auto label = CreateWindowEx(0, L"STATIC", L"Resolution:",
            WS_CHILD | WS_VISIBLE,
            20, 20, 80, 20,
            _parent, nullptr, GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(label);

        _combo_resolution = CreateWindowEx(0, L"COMBOBOX", nullptr,
            CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
            110, 17, 150, 100,
            _parent, nullptr, GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(_combo_resolution);
        AddResolutionOptions();

        // VSync CheckBox
        _check_vsync = CreateWindowEx(0, L"BUTTON", L"VSync Enabled?",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 60, 200, 20,
            _parent, nullptr, GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(_check_vsync);

        // Limit Framerate ComboBox
        label = CreateWindowEx(0, L"STATIC", L"FPS:",
            WS_CHILD | WS_VISIBLE,
            20, 100, 100, 20,
            _parent, nullptr, GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(label);

        _combo_framerate = CreateWindowEx(0, L"COMBOBOX", nullptr,
            CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE,
            110, 97, 120, 100,
            _parent, nullptr, GetModuleHandle(nullptr), nullptr);
        uiStyle.ApplyUIFont(_combo_framerate);
        AddFramerateOptions();

        LoadSettings();
    }

    void GraphicsSettingsUI::AddResolutionOptions() const
    {
        for (const auto& option : kResolutionOptions)
        {
            SendMessage(_combo_resolution, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(option.label));
        }
        SendMessage(_combo_resolution, CB_SETCURSEL, 0, 0);
    }

    void GraphicsSettingsUI::AddFramerateOptions() const
    {
        for (const auto& option : kFramerateOptions)
        {
            SendMessage(_combo_framerate, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(option.label));
        }
        SendMessage(_combo_framerate, CB_SETCURSEL, 1, 0);
    }

    void GraphicsSettingsUI::ApplySettings() const
    {
        config::GraphicsConfig config{};
        config.resolutionIndex = static_cast<int>(SendMessage(_combo_resolution, CB_GETCURSEL, 0, 0));
        config.vsync = Button_GetCheck(_check_vsync);
        config.fpsLimitIndex = static_cast<int>(SendMessage(_combo_framerate, CB_GETCURSEL, 0, 0));

        config::ConfigUIManager::SaveGraphicsConfig(config);

        // Changes the resolution
        core::winapi::WindowManager::GetInstance().ChangeWindowSize(kResolutionOptions[config.resolutionIndex].scale);

        // Changes the frame rate limit
        if (config.fpsLimitIndex >= 0 && config.fpsLimitIndex < static_cast<int>(std::size(kFramerateOptions)))
        {
            utils::FpsManager::GetInstance().SetTargetFps(kFramerateOptions[config.fpsLimitIndex].targetFps);
        }
    }

    void GraphicsSettingsUI::LoadSettings() const
    {
        config::GraphicsConfig config{};
        config::ConfigUIManager::LoadGraphicsConfig(config);

        SendMessage(_combo_resolution, CB_SETCURSEL, config.resolutionIndex, 0);
        Button_SetCheck(_check_vsync, config.vsync ? BST_CHECKED : BST_UNCHECKED);
        SendMessage(_combo_framerate, CB_SETCURSEL, config.fpsLimitIndex, 0);
    }
}