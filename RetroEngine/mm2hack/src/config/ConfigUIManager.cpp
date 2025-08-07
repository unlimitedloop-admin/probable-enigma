#include "pch.h"

#include "ConfigUIManager.h"

#include <cstdlib>
#include "config/GraphicsConfig.h"
#include "config/HudConfig.h"
#include "config/SoundConfig.h"

namespace mm2hack::config
{
    std::wstring ConfigUIManager::GetIniPath()
    {
        return L"./settings.ini";
    }

    void ConfigUIManager::SaveGraphicsConfig(const GraphicsConfig& config)
    {
        const std::wstring path = GetIniPath();

        WritePrivateProfileString(L"Graphics", L"ResolutionIndex", std::to_wstring(config.resolutionIndex).c_str(), path.c_str());
        WritePrivateProfileString(L"Graphics", L"VSync", config.vsync ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Graphics", L"FpsLimit", std::to_wstring(config.fpsLimitIndex).c_str(), path.c_str());
    }

    void ConfigUIManager::LoadGraphicsConfig(GraphicsConfig& config)
    {
        const std::wstring path = GetIniPath();
        wchar_t buffer[32];

        GetPrivateProfileString(L"Graphics", L"ResolutionIndex", L"0", buffer, 32, path.c_str());
        config.resolutionIndex = _wtoi(buffer);

        GetPrivateProfileString(L"Graphics", L"VSync", L"1", buffer, 32, path.c_str());
        config.vsync = (_wtoi(buffer) != 0);

        GetPrivateProfileString(L"Graphics", L"FpsLimit", L"1", buffer, 32, path.c_str());
        config.fpsLimitIndex = _wtoi(buffer);
    }

    void ConfigUIManager::SaveSoundConfig(const SoundConfig& config)
    {
        const std::wstring path = GetIniPath();

        WritePrivateProfileString(L"Sound", L"Master", std::to_wstring(config.master).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"BGM", std::to_wstring(config.bgm).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"SE", std::to_wstring(config.se).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"Enabled", config.enabled ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Sound", L"Source", std::to_wstring(config.sourceIndex).c_str(), path.c_str());
    }

    void ConfigUIManager::LoadSoundConfig(SoundConfig& config)
    {
        const std::wstring path = GetIniPath();
        wchar_t buffer[32];

        GetPrivateProfileString(L"Sound", L"Master", L"80", buffer, 32, path.c_str());
        config.master = _wtoi(buffer);

        GetPrivateProfileString(L"Sound", L"BGM", L"80", buffer, 32, path.c_str());
        config.bgm = _wtoi(buffer);

        GetPrivateProfileString(L"Sound", L"SE", L"80", buffer, 32, path.c_str());
        config.se = _wtoi(buffer);

        GetPrivateProfileString(L"Sound", L"Enabled", L"1", buffer, 32, path.c_str());
        config.enabled = (_wtoi(buffer) != 0);

        GetPrivateProfileString(L"Sound", L"Source", L"0", buffer, 32, path.c_str());
        config.sourceIndex = _wtoi(buffer);
    }

    HudConfig ConfigUIManager::_cachedHudConfig{ false };

    void ConfigUIManager::SaveHudConfig(const HudConfig& config)
    {
        const std::wstring path = GetIniPath();
        WritePrivateProfileString(L"Hud", L"ShowFps", config.showFps ? L"1" : L"0", path.c_str());
    }

    void ConfigUIManager::LoadHudConfig(HudConfig& config)
    {
        const std::wstring path = GetIniPath();
        wchar_t buffer[32];
        GetPrivateProfileString(L"Hud", L"ShowFps", L"0", buffer, 32, path.c_str());
        config.showFps = (_wtoi(buffer) != 0);

        // Cache the loaded configuration.
        _cachedHudConfig = config;
    }

    const HudConfig& ConfigUIManager::GetCurrentHudConfig()
    {
        return _cachedHudConfig;
    }

    void ConfigUIManager::SetCurrentHudConfig(const HudConfig& config)
    {
        _cachedHudConfig = config;
        SaveHudConfig(_cachedHudConfig);
    }
}