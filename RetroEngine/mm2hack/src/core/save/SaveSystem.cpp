#include "pch.h"

#include "SaveSystem.h"

#include <array>
#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <limits>
#include "SaveData.h"
#include "StateIO.h"

namespace mm2hack::core::save
{
    namespace fs = std::filesystem;

    namespace
    {
        constexpr std::array<std::uint8_t, 8> kSaveMagic{
            'M', 'M', '2', 'S', 'A', 'V', 'E', 0
        };
    }

    bool SaveSystem::Save(const std::wstring& path, const SaveData& data)
    {
        if (data.scenePayload.size() > kMaximumPayloadSize ||
            data.scenePayload.size() > std::numeric_limits<std::uint32_t>::max())
        {
            return false;
        }

        try
        {
            const fs::path savepath(path);
            if (const auto parent = savepath.parent_path(); !parent.empty())
            {
                fs::create_directories(parent);
            }
        }
        catch (const fs::filesystem_error&)
        {
            return false;
        }

        std::ofstream ofs(path, std::ios::binary);
        if (!ofs)
        {
            return false;
        }

        StateWriter writer(ofs);
        const bool wroteAll =
            writer.WriteBytes(kSaveMagic) &&
            writer.WriteU32(config::SystemConfig::kCurrentSaveVersion) &&
            writer.WriteI32(data.sequenceID) &&
            writer.WriteI32(data.sceneID) &&
            writer.WriteU32(static_cast<std::uint32_t>(data.scenePayload.size())) &&
            writer.WriteBytes(data.scenePayload);
        ofs.flush();
        return wroteAll && ofs.good();
    }

    bool SaveSystem::Load(const std::wstring& path, SaveData& outData)
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs)
        {
            return false;
        }

        StateReader reader(ifs);

        std::array<std::uint8_t, kSaveMagic.size()> magic{};
        std::uint32_t fileVersion{};
        std::int32_t sequenceID{};
        std::int32_t sceneID{};
        std::uint32_t payloadSize{};

        if (!reader.ReadBytes(magic) || magic != kSaveMagic ||
            !reader.ReadU32(fileVersion) ||
            fileVersion != config::SystemConfig::kCurrentSaveVersion ||
            !reader.ReadI32(sequenceID) ||
            !reader.ReadI32(sceneID) ||
            !reader.ReadU32(payloadSize) ||
            payloadSize > kMaximumPayloadSize)
        {
            return false;
        }

        SaveData loaded{};
        loaded.sequenceID = sequenceID;
        loaded.sceneID = sceneID;
        loaded.scenePayload.resize(payloadSize);
        if (!reader.ReadBytes(loaded.scenePayload))
        {
            return false;
        }

        if (ifs.peek() != std::char_traits<char>::eof())
        {
            return false;
        }

        outData = std::move(loaded);
        return true;
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
