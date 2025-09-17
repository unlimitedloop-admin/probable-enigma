#include "pch.h"

#include "GraphicsSettingsUI.h"

#include <cmath>
#include <CommCtrl.h>
#include <iterator>
#include <libloaderapi.h>
#include <limits>
#include <Windowsx.h>
#include "CommonUIStyle.h"
#include "config/ConfigUIManager.h"
#include "config/GraphicsConfig.h"
#include "core/winapi/WindowManager.h"
#include "utils/FpsManager.h"

namespace mm2hack::core::ui
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
            CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
            110, 97, 120, 210,
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
        SendMessage(_combo_framerate, CB_SETMINVISIBLE, 6, 0);
    }

    void GraphicsSettingsUI::ApplySettings() const
    {
        config::GraphicsConfig config{};
        config.resolutionIndex = static_cast<int>(SendMessage(_combo_resolution, CB_GETCURSEL, 0, 0));
        config.vsync = Button_GetCheck(_check_vsync);
        config.fpsLimitIndex = static_cast<int>(SendMessage(_combo_framerate, CB_GETCURSEL, 0, 0));

        config::ConfigUIManager::SaveGraphicsConfig(config);

        // Changes the resolution
        if (config.resolutionIndex >= 0 && config.resolutionIndex < static_cast<int>(std::size(kResolutionOptions)))
        {
            core::winapi::WindowManager::GetInstance().ChangeWindowSize(kResolutionOptions[config.resolutionIndex].scale);
        }

        // VSync control = It follows the refresh rate of the monitor.
        core::winapi::WindowManager::GetInstance().SetVSyncEnabled(config.vsync);

        int fpsToSet = 0;
        if (!config.vsync && config.fpsLimitIndex >= 0 && config.fpsLimitIndex < static_cast<int>(std::size(kFramerateOptions)))
        {
            fpsToSet = kFramerateOptions[config.fpsLimitIndex].targetFps;   // Set the specified FPS limit.
        }

        utils::FpsManager::GetInstance().SetTargetFps(fpsToSet);
    }

    void GraphicsSettingsUI::LoadSettings()
    {
        using namespace config;

        const auto findResolutionIndexByScale = [](float scale, float eps = 0.01f) -> int
            {
                for (int i = 0; i < static_cast<int>(std::size(kResolutionOptions)); ++i)
                {
                    if (std::fabs(kResolutionOptions[i].scale - scale) < eps) return i;
                }
                return -1;
            };

        const auto sanitizeIndex = [](int idx, int size, int fallback) -> int
            {
                return (0 <= idx && idx < size) ? idx : fallback;
            };

        GraphicsConfig cfg{};
        ConfigUIManager::LoadGraphicsConfig(cfg);   // Load from INI.

        // --- Resolution: INI -> If not set/out of range, go to SystemConfig default -> Overwrite with runtime viewerRate if present ---
        const int defaultResIdx = [&]
            {
                int idx = findResolutionIndexByScale(SystemConfig::kScreenScale);
                return (idx >= 0) ? idx : 0; // If not exists set kScreenScale.
            }();

        int resolutionIndex = sanitizeIndex(
            cfg.resolutionIndex,
            static_cast<int>(std::size(kResolutionOptions)),
            defaultResIdx
        );

        if (int runtimeIdx = findResolutionIndexByScale(winapi::WindowManager::GetInstance().GetViewerRate());
            runtimeIdx >= 0)
        {
            // If the current resolution matches one of the predefined options, select it.
            resolutionIndex = runtimeIdx;
        }
        SendMessage(_combo_resolution, CB_SETCURSEL, resolutionIndex, 0);

        // --- VSync ---
        Button_SetCheck(_check_vsync, cfg.vsync ? BST_CHECKED : BST_UNCHECKED);

        // --- FPS: INI -> If not set/out of range, go to SystemConfig default (or nearest) ---
        const auto sanitizeFpsIndex = [&](int idx) -> int
            {
                const int size = static_cast<int>(std::size(kFramerateOptions));
                if (0 <= idx && idx < size) return idx;

                constexpr int kDefaultFps = SystemConfig::kTargetFps;
                int mapped = MapFpsToIndex(kDefaultFps);
                if (mapped < 0) mapped = MapFpsToNearestIndex(kDefaultFps);
                return mapped;
            };

        const int fpsIndex = sanitizeFpsIndex(cfg.fpsLimitIndex);
        SendMessage(_combo_framerate, CB_SETCURSEL, fpsIndex, 0);
    }

    int GraphicsSettingsUI::MapFpsToIndex(int fps) noexcept
    {
        for (int i = 0; i < static_cast<int>(std::size(kFramerateOptions)); ++i)
        {
            if (kFramerateOptions[i].targetFps == fps) return i;
        }
        return -1;
    }

    int GraphicsSettingsUI::MapFpsToNearestIndex(int fps) noexcept
    {
        if (fps <= 0)
        {
            // "not restricted" i.e. "0".
            return MapFpsToIndex(0);
        }
        int bestIdx = 0;
        int bestDiff = std::numeric_limits<int>::max();
        for (int i = 0; i < static_cast<int>(std::size(kFramerateOptions)); ++i)
        {
            int diff = std::abs(kFramerateOptions[i].targetFps - fps);
            if (diff < bestDiff) { bestDiff = diff; bestIdx = i; }
        }
        return bestIdx;
    }
}