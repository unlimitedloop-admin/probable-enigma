#include "pch.h"

#include "ConfigUIManager.h"

#include <cstdio>
#include <cstdlib>
#include "GraphicsConfig.h"
#include "HudConfig.h"
#include "input/Jpbtn.h"
#include "input/KeyBinding.h"
#include "input/KeyToken.h"
#include "InputLoadResult.h"
#include "SoundConfig.h"

namespace
{
    namespace inp = mm2hack::input;

    inline uint16_t ParseU16(const wchar_t* ws)
    {
        if (!ws || !*ws) return inp::kTokenUnbound;
        wchar_t* end = nullptr;
        unsigned long v = std::wcstoul(ws, &end, 0); // 0xFFFF, 65535 ...
        if (v > 0xFFFFul) v = 0xFFFFul;
        return static_cast<uint16_t>(v);
    }
    inline std::wstring ToStrU16(uint16_t v, bool hex = false)
    {
        if (hex) { wchar_t buf[16]; std::swprintf(buf, 16, L"0x%04X", (unsigned)v); return buf; }
        return std::to_wstring((unsigned)v);
    }

    // Translate Device enum to string and vice versa
    inline const wchar_t* DeviceToW(inp::Device d)
    {
        using D = inp::Device;
        switch (d) { case D::XInput: return L"XInput"; case D::DirectInput: return L"DirectInput"; default: return L"Keyboard"; }
    }

    // Parse device string to Device enum
    inline inp::Device ParseDevice(const wchar_t* ws)
    {
        using D = inp::Device;
        if (!ws) return D::Keyboard;
        if (_wcsicmp(ws, L"XInput") == 0)      return D::XInput;
        if (_wcsicmp(ws, L"DirectInput") == 0) return D::DirectInput;
        if (_wcsicmp(ws, L"Keyboard") == 0)    return D::Keyboard;
        int n = _wtoi(ws);
        if (n == 1) return D::XInput; if (n == 2) return D::DirectInput; return D::Keyboard;
    }

    inline bool ProviderMatches(inp::Device saved, inp::Device detected)
    {
        return saved == detected;
    }
}


namespace mm2hack::config
{
    HudConfig ConfigUIManager::_cachedHudConfig{ false };

    void ConfigUIManager::SaveInputDeviceConfig(const KeyBinding& binding, Device provider)
    {
        const std::wstring path = getIniPath_();
        constexpr bool kHex = false;

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const auto tok = binding.GetBindingSCon(static_cast<JPBTN>(i));
            WritePrivateProfileString(L"Input", inp::kJpbtnKeys[i], ToStrU16(tok, kHex).c_str(), path.c_str());
        }

        WritePrivateProfileString(L"Input", L"XInputEnabled", binding.IsXInputEnabled() ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Input", L"HatSwitchEnabled", binding.IsHatSwitchEnabled() ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Input", L"TriggerEnabled", binding.IsTriggerEnabled() ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Input", L"ThumbEnabled", binding.IsThumbEnabled() ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Input", L"Provider", DeviceToW(provider), path.c_str());
    }

    ConfigUIManager::Device ConfigUIManager::LoadInputDeviceConfig(KeyBinding& binding)
    {
        const std::wstring path = getIniPath_();
        wchar_t buf[64];

        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const uint16_t cur = binding.GetBindingSCon(static_cast<JPBTN>(i));
            GetPrivateProfileString(L"Input", inp::kJpbtnKeys[i], ToStrU16(cur, false).c_str(), buf, 64, path.c_str());
            binding.SetBinding(static_cast<JPBTN>(i), ParseU16(buf));
        }

        auto getBool = [&](const wchar_t* key, bool def)->bool
            {
                GetPrivateProfileString(L"Input", key, def ? L"1" : L"0", buf, 64, path.c_str());
                return _wtoi(buf) != 0;
            };
        const bool fx = getBool(L"XInputEnabled", binding.IsXInputEnabled());
        const bool fhs = getBool(L"HatSwitchEnabled", binding.IsHatSwitchEnabled());
        const bool ftg = getBool(L"TriggerEnabled", binding.IsTriggerEnabled());
        const bool fth = getBool(L"ThumbEnabled", binding.IsThumbEnabled());
        binding.SetFeatureFlags(fx, fhs, ftg, fth);

        GetPrivateProfileString(L"Input", L"Provider", L"Keyboard", buf, 64, path.c_str());
        return ParseDevice(buf);
    }

    InputLoadResult ConfigUIManager::LoadInputConfigIfMatches(KeyBinding& binding, Device detected)
    {
        const std::wstring path = getIniPath_();
        wchar_t buf[64];

        // Load the saved provider and check if it matches the detected one.
        GetPrivateProfileString(L"Input", L"Provider", L"", buf, 64, path.c_str());
        const Device savedProv = (buf[0] ? ParseDevice(buf) : detected); // Default to detected if not found
        const bool match = ProviderMatches(savedProv, detected);

        // One by one, load the button mappings (default to current values if not found)
        std::array<uint16_t, JPBTN_COUNT> tmp{};
        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            const uint16_t cur = binding.GetBindingSCon(static_cast<JPBTN>(i));
            GetPrivateProfileString(L"Input", inp::kJpbtnKeys[i], ToStrU16(cur).c_str(), buf, 64, path.c_str());
            tmp[i] = ParseU16(buf);
        }
        auto getBool = [&](const wchar_t* key, bool def)->bool
            {
                GetPrivateProfileString(L"Input", key, def ? L"1" : L"0", buf, 64, path.c_str());
                return _wtoi(buf) != 0;
            };
        const bool fx = getBool(L"XInputEnabled", binding.IsXInputEnabled());
        const bool fhs = getBool(L"HatSwitchEnabled", binding.IsHatSwitchEnabled());
        const bool ftg = getBool(L"TriggerEnabled", binding.IsTriggerEnabled());
        const bool fth = getBool(L"ThumbEnabled", binding.IsThumbEnabled());

        // If not matched, return without applying
        if (!match)
        {
            return { false, savedProv };
        }

        // Apply the loaded configuration
        for (size_t i = 0; i < JPBTN_COUNT; ++i)
        {
            binding.SetBinding(static_cast<JPBTN>(i), tmp[i]);
        }
        binding.SetFeatureFlags(fx, fhs, ftg, fth);
        return { true, savedProv };
    }

    void ConfigUIManager::SaveGraphicsConfig(const GraphicsConfig& config)
    {
        const std::wstring path = getIniPath_();

        WritePrivateProfileString(L"Graphics", L"ResolutionIndex", std::to_wstring(config.resolutionIndex).c_str(), path.c_str());
        WritePrivateProfileString(L"Graphics", L"VSync", config.vsync ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Graphics", L"FpsLimit", std::to_wstring(config.fpsLimitIndex).c_str(), path.c_str());
    }

    void ConfigUIManager::LoadGraphicsConfig(GraphicsConfig& config)
    {
        const std::wstring path = getIniPath_();
        wchar_t buffer[32];

        GetPrivateProfileString(L"Graphics", L"ResolutionIndex", L"-1", buffer, 32, path.c_str());
        config.resolutionIndex = _wtoi(buffer);

        GetPrivateProfileString(L"Graphics", L"VSync", L"1", buffer, 32, path.c_str());
        config.vsync = (_wtoi(buffer) != 0);

        GetPrivateProfileString(L"Graphics", L"FpsLimit", L"-1", buffer, 32, path.c_str());
        config.fpsLimitIndex = _wtoi(buffer);
    }

    void ConfigUIManager::SaveSoundConfig(const SoundConfig& config)
    {
        const std::wstring path = getIniPath_();

        WritePrivateProfileString(L"Sound", L"Master", std::to_wstring(config.master).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"BGM", std::to_wstring(config.bgm).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"SE", std::to_wstring(config.se).c_str(), path.c_str());
        WritePrivateProfileString(L"Sound", L"Enabled", config.enabled ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Sound", L"Source", std::to_wstring(config.sourceIndex).c_str(), path.c_str());
    }

    void ConfigUIManager::LoadSoundConfig(SoundConfig& config)
    {
        const std::wstring path = getIniPath_();
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

    void ConfigUIManager::SaveHudConfig(const HudConfig& config)
    {
        const std::wstring path = getIniPath_();
        WritePrivateProfileString(L"Hud", L"ShowFps", config.showFps ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Hud", L"ShowFrameTime", config.showFrameTime ? L"1" : L"0", path.c_str());
        WritePrivateProfileString(L"Hud", L"ShowScrollLine", config.showScrollLine ? L"1" : L"0", path.c_str());
    }

    void ConfigUIManager::LoadHudConfig(HudConfig& config)
    {
        const std::wstring path = getIniPath_();
        wchar_t buffer[32];
        GetPrivateProfileString(L"Hud", L"ShowFps", L"0", buffer, 32, path.c_str());
        config.showFps = (_wtoi(buffer) != 0);
        GetPrivateProfileString(L"Hud", L"ShowFrameTime", L"0", buffer, 32, path.c_str());
        config.showFrameTime = (_wtoi(buffer) != 0);
        GetPrivateProfileString(L"Hud", L"ShowScrollLine", L"0", buffer, 32, path.c_str());
        config.showScrollLine = (_wtoi(buffer) != 0);

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

    std::wstring ConfigUIManager::getIniPath_()
    {
        return _kIniFileName;
    }
}