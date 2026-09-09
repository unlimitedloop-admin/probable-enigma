#include "pch.h"

#include "SaveSystem.h"

#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <span>
#include <string_view>
#include <system_error>

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
        constexpr std::uint32_t kCrc32Polynomial = 0xEDB88320U;

        void AccumulateCrc32(std::uint32_t& checksum, std::uint8_t value) noexcept
        {
            checksum ^= value;
            for (int bit = 0; bit < 8; ++bit)
            {
                const std::uint32_t mask =
                    0U - static_cast<std::uint32_t>(checksum & 1U);
                checksum = (checksum >> 1) ^ (kCrc32Polynomial & mask);
            }
        }

        void AccumulateCrc32(std::uint32_t& checksum, std::uint32_t value) noexcept
        {
            for (int byte = 0; byte < 4; ++byte)
            {
                AccumulateCrc32(checksum, static_cast<std::uint8_t>(value));
                value >>= 8;
            }
        }

        std::uint32_t ComputeSaveChecksum(
            std::int32_t sequence_id,
            std::int32_t scene_id,
            std::span<const std::uint8_t> payload) noexcept
        {
            std::uint32_t checksum = 0xFFFFFFFFU;
            AccumulateCrc32(checksum, std::bit_cast<std::uint32_t>(sequence_id));
            AccumulateCrc32(checksum, std::bit_cast<std::uint32_t>(scene_id));
            AccumulateCrc32(checksum, static_cast<std::uint32_t>(payload.size()));
            for (const std::uint8_t byte : payload)
            {
                AccumulateCrc32(checksum, byte);
            }
            return ~checksum;
        }
    }

    bool SaveSystem::Save(const std::wstring& path, const SaveData& data)
    {
        if (data.scenePayload.size() > kMaximumPayloadSize ||
            data.scenePayload.size() > std::numeric_limits<std::uint32_t>::max())
        {
            return false;
        }

        const fs::path savepath(path);
        fs::path temporary_path = savepath;
        temporary_path += L".tmp";

        try
        {
            if (const auto parent = savepath.parent_path(); !parent.empty())
            {
                fs::create_directories(parent);
            }
        }
        catch (const fs::filesystem_error&)
        {
            return false;
        }

        std::ofstream ofs(temporary_path, std::ios::binary | std::ios::trunc);
        if (!ofs)
        {
            return false;
        }

        StateWriter writer(ofs);
        const std::uint32_t checksum = ComputeSaveChecksum(
            data.sequenceID,
            data.sceneID,
            data.scenePayload);
        const bool wroteAll =
            writer.WriteBytes(kSaveMagic) &&
            writer.WriteU32(config::SystemConfig::kCurrentSaveVersion) &&
            writer.WriteI32(data.sequenceID) &&
            writer.WriteI32(data.sceneID) &&
            writer.WriteU32(static_cast<std::uint32_t>(data.scenePayload.size())) &&
            writer.WriteU32(checksum) &&
            writer.WriteBytes(data.scenePayload);
        ofs.flush();
        const bool flushed = ofs.good();
        ofs.close();
        if (!wroteAll || !flushed || ofs.fail())
        {
            std::error_code ignored;
            fs::remove(temporary_path, ignored);
            return false;
        }

        if (!::MoveFileExW(
            temporary_path.c_str(),
            savepath.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        {
            std::error_code ignored;
            fs::remove(temporary_path, ignored);
            return false;
        }
        return true;
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
        std::uint32_t expectedChecksum{};

        if (!reader.ReadBytes(magic) || magic != kSaveMagic ||
            !reader.ReadU32(fileVersion) ||
            fileVersion != config::SystemConfig::kCurrentSaveVersion ||
            !reader.ReadI32(sequenceID) ||
            !reader.ReadI32(sceneID) ||
            !reader.ReadU32(payloadSize) ||
            payloadSize > kMaximumPayloadSize ||
            !reader.ReadU32(expectedChecksum))
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

        if (ComputeSaveChecksum(sequenceID, sceneID, loaded.scenePayload) !=
            expectedChecksum ||
            ifs.peek() != std::char_traits<char>::eof())
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
