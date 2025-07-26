#include "pch.h"

#include "ConfigUIManager.h"

#include <cstdlib>

namespace mm2hack::config
{
    std::wstring ConfigManager::GetIniPath()
    {
        return L"./settings.ini";
    }

    void ConfigManager::SaveGraphicsConfig(const GraphicsConfig& config)
    {
        const std::wstring path = GetIniPath();

        WritePrivateProfileString(L"Graphics", L"ResolutionIndex", std::to_wstring(config.resolutionIndex).c_str(), path.c_str());
        WritePrivateProfileString(L"Graphics", L"VSync", config.vsync ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Graphics", L"FpsLimit", std::to_wstring(config.fpsLimitIndex).c_str(), path.c_str());
    }

    void ConfigManager::LoadGraphicsConfig(GraphicsConfig& config)
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

    void ConfigManager::SaveSoundConfig(const SoundConfig& config)
    {
        const std::wstring path = GetIniPath();

        WritePrivateProfileString(L"Sound", L"Master", std::to_wstring(config.master).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"BGM", std::to_wstring(config.bgm).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"SE", std::to_wstring(config.se).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"Enabled", config.enabled ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Sound", L"Source", std::to_wstring(config.sourceIndex).c_str(), path.c_str());
    }

    void ConfigManager::LoadSoundConfig(SoundConfig& config)
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
}