#include "EnvironmentConfig.h"

#include <fstream>
#include <string>
#include <unordered_map>

namespace mm2hack::config
{
    std::unordered_map<std::wstring, std::wstring> EnvironmentConfig::_values;

    void EnvironmentConfig::LoadFromFile(const std::wstring& filePath)
    {
        std::wifstream file(filePath);
        if (!file.is_open()) return;

        std::wstring line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == L'#')
                continue;

            size_t keyStart = line.find(L'$');
            if (keyStart != 0) continue;

            size_t spacePos = line.find(L' ');
            if (spacePos == std::wstring::npos) continue;

            std::wstring key = line.substr(1, spacePos - 1);
            std::wstring value = line.substr(spacePos + 1);
            _values[key] = Trim(value);
        }
    }

    std::wstring EnvironmentConfig::Get(const std::wstring& key, const std::wstring& defaultValue)
    {
        auto it = _values.find(key);
        return (it != _values.end()) ? it->second : defaultValue;
    }

    int EnvironmentConfig::GetInt(const std::wstring& key, int defaultValue)
    {
        auto val = Get(key);
        return val.empty() ? defaultValue : std::stoi(val);
    }

    bool EnvironmentConfig::GetBool(const std::wstring& key, bool defaultValue)
    {
        auto val = Get(key);
        return (val == L"1" || val == L"true") ? true :
            (val == L"0" || val == L"false") ? false : defaultValue;
    }

    std::wstring EnvironmentConfig::Trim(const std::wstring& str)
    {
        const wchar_t* whitespace = L" \t\n\r";
        size_t start = str.find_first_not_of(whitespace);
        size_t end = str.find_last_not_of(whitespace);
        return (start == std::wstring::npos || end == std::wstring::npos)
            ? L"" : str.substr(start, end - start + 1);
    }
}