#include "SaveSystem.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include "config/SystemConfig.h"
#include "SaveData.h"

namespace mm2hack::core::save
{
    namespace fs = std::filesystem;

    bool SaveSystem::Save(const std::wstring& path, const SaveData& data)
    {
        fs::path savepath(path);
        fs::create_directories(savepath.parent_path());

        std::ofstream ofs(path, std::ios::binary);
        if (!ofs)
        {
            return false;
        }

        // Save version.
        uint32_t fileVersion = config::SystemConfig::kCurrentSaveVersion;
        ofs.write(reinterpret_cast<const char*>(&fileVersion), sizeof(fileVersion));

        // Save full data structure.
        ofs.write(reinterpret_cast<const char*>(&data), sizeof(SaveData));

        return ofs.good();
    }

    bool SaveSystem::Load(const std::wstring& path, SaveData& outData)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs)
        {
            return false;
        }

        // Load and check version.
        uint32_t fileVersion = 0;
        ifs.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
        if (fileVersion != config::SystemConfig::kCurrentSaveVersion)
        {
            return false;   // Version mismatch.
        }

        // Load full data structure.
        ifs.read(reinterpret_cast<char*>(&outData), sizeof(SaveData));

        return ifs.good();
    }

    void SaveSystem::SetCurrentSlot(int slot)
    {
        _currentSlot = std::clamp(slot, 0, 9);
    }

    int SaveSystem::GetCurrentSlot()
    {
        return _currentSlot;
    }

    std::wstring SaveSystem::GetCurrentSlotFilename()
    {
        wchar_t buffer[32];
        swprintf_s(buffer, L"sav/slot%02d.sav", _currentSlot);
        return std::wstring(buffer);
    }
}