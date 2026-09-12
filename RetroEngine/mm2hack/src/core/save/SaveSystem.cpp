#include "pch.h"

#include "SaveSystem.h"

#include <bit>
#include <cstdio>
#include <filesystem>
#include <istream>
#include <limits>
#include <span>
#include <string_view>
#include <system_error>

#include "config/SystemConfig.h"
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

        void AccumulateCrc32(std::uint32_t& checksum, std::uint64_t value) noexcept
        {
            for (int byte = 0; byte < 8; ++byte)
            {
                AccumulateCrc32(checksum, static_cast<std::uint8_t>(value));
                value >>= 8;
            }
        }

        std::uint32_t ComputeSaveChecksum(
            std::uint64_t compatibility_id,
            std::int32_t sequence_id,
            std::int32_t scene_id,
            std::span<const std::uint8_t> payload) noexcept
        {
            std::uint32_t checksum = 0xFFFFFFFFU;
            AccumulateCrc32(checksum, compatibility_id);
            AccumulateCrc32(checksum, std::bit_cast<std::uint32_t>(sequence_id));
            AccumulateCrc32(checksum, std::bit_cast<std::uint32_t>(scene_id));
            AccumulateCrc32(checksum, static_cast<std::uint32_t>(payload.size()));
            for (const std::uint8_t byte : payload)
            {
                AccumulateCrc32(checksum, byte);
            }
            return ~checksum;
        }

        LoadResult ClassifyReadFailure(const std::istream& stream) noexcept
        {
            return stream.bad() ? LoadResult::IoError : LoadResult::Corrupt;
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
            data.gameContentCompatibilityId,
            data.sequenceID,
            data.sceneID,
            data.scenePayload);
        const bool wroteAll =
            writer.WriteBytes(kSaveMagic) &&
            writer.WriteU32(config::SystemConfig::kCurrentSaveVersion) &&
            writer.WriteU64(data.gameContentCompatibilityId) &&
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

    LoadResult SaveSystem::Load(const std::wstring& path, SaveData& outData)
    {
        std::error_code path_error;
        if (!fs::exists(path, path_error))
        {
            return path_error ? LoadResult::IoError : LoadResult::FileNotFound;
        }
        if (!fs::is_regular_file(path, path_error) || path_error)
        {
            return LoadResult::IoError;
        }

        std::ifstream ifs(path, std::ios::binary);
        if (!ifs)
        {
            return LoadResult::IoError;
        }

        StateReader reader(ifs);

        std::array<std::uint8_t, kSaveMagic.size()> magic{};
        std::uint32_t fileVersion{};
        std::uint64_t compatibilityId{};
        std::int32_t sequenceID{};
        std::int32_t sceneID{};
        std::uint32_t payloadSize{};
        std::uint32_t expectedChecksum{};

        if (!reader.ReadBytes(magic))
        {
            return ClassifyReadFailure(ifs);
        }
        if (magic != kSaveMagic)
        {
            return LoadResult::Corrupt;
        }
        if (!reader.ReadU32(fileVersion))
        {
            return ClassifyReadFailure(ifs);
        }
        if (fileVersion != config::SystemConfig::kCurrentSaveVersion)
        {
            return LoadResult::UnsupportedVersion;
        }
        if (!reader.ReadU64(compatibilityId) ||
            !reader.ReadI32(sequenceID) ||
            !reader.ReadI32(sceneID) ||
            !reader.ReadU32(payloadSize))
        {
            return ClassifyReadFailure(ifs);
        }
        if (payloadSize > kMaximumPayloadSize)
        {
            return LoadResult::Corrupt;
        }
        if (!reader.ReadU32(expectedChecksum))
        {
            return ClassifyReadFailure(ifs);
        }

        SaveData loaded{};
        loaded.gameContentCompatibilityId = compatibilityId;
        loaded.sequenceID = sequenceID;
        loaded.sceneID = sceneID;
        loaded.scenePayload.resize(payloadSize);
        if (!reader.ReadBytes(loaded.scenePayload))
        {
            return ClassifyReadFailure(ifs);
        }

        if (ComputeSaveChecksum(
            compatibilityId,
            sequenceID,
            sceneID,
            loaded.scenePayload) !=
            expectedChecksum)
        {
            return LoadResult::Corrupt;
        }

        const int trailing_byte = ifs.peek();
        if (ifs.bad())
        {
            return LoadResult::IoError;
        }
        if (trailing_byte != std::char_traits<char>::eof())
        {
            return LoadResult::Corrupt;
        }
        if (compatibilityId != config::SystemConfig::kGameContentCompatibilityId)
        {
            return LoadResult::IncompatibleContent;
        }

        outData = std::move(loaded);
        return LoadResult::Success;
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
