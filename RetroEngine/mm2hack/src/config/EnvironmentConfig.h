//==============================================================================
// 
//  Project: mm2hack
//  EnvironmentConfig.h
// 
//  Loads configuration settings from a file and provides access to them.
// 
//==============================================================================
#pragma once

#include <string>
#include <unordered_map>

namespace mm2hack::config
{
    class EnvironmentConfig final
    {
    public:
        EnvironmentConfig() = delete;
        ~EnvironmentConfig() = delete;
        EnvironmentConfig(const EnvironmentConfig&) = delete;
        EnvironmentConfig& operator=(const EnvironmentConfig&) = delete;
        EnvironmentConfig(EnvironmentConfig&&) = delete;
        EnvironmentConfig& operator=(EnvironmentConfig&&) = delete;
        // This class is not copyable or movable (static class)

        // Load configuration from a file
        static void LoadFromFile(const std::wstring& filePath);
        // Get configuration values
        static std::wstring Get(const std::wstring& key, const std::wstring& defaultValue = L"");
        // Get configuration values as int or bool
        static int GetInt(const std::wstring& key, int defaultValue = 0);
        // Get configuration values as bool
        static bool GetBool(const std::wstring& key, bool defaultValue = false);

    private:
        static std::unordered_map<std::wstring, std::wstring> _values;
        static std::wstring Trim(const std::wstring& str);
    };
}