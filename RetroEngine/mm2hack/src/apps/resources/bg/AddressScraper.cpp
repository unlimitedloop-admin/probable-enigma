#include "pch.h"

#include "AddressScraper.h"

namespace
{
    using namespace mm2hack::config;
    using namespace mm2hack::apps::resources::bg;

    constexpr std::size_t kPageSize = SystemConfig::kMapBinaryUnitPageSize;
    constexpr std::size_t kHeaderSize = SystemConfig::kMapBinaryHeaderSize; // [0x00..0x0F] inclusive (16B, unchanged by BD-005)
    constexpr std::size_t kPayloadOff = SystemConfig::kMapBinaryHeaderSize;
    constexpr std::size_t kPayloadEnd = kPageSize - kHeaderSize;            // [0x10..0xFF] inclusive
    constexpr std::size_t kPayloadSize = kPayloadEnd;                       // 240B

    // No-neighbor sentinel per BD-005 (was 0x00 under the old layout)
    constexpr std::uint8_t kNoNeighbor = 0xFF;
}

namespace mm2hack::apps::resources::bg
{
    AddressScraper::AddressScraper(std::vector<std::uint8_t> bin)
        : _bin(std::move(bin))
    {
        // Minimum integrity check (multiple of page size)
        if (_bin.empty() || (_bin.size() % kPageSize) != 0)
        {
            _bin.clear();
        }
        buildPageIndexMap_();
    }

    int16_t AddressScraper::getPageIndex(std::size_t roomNo) const
    {
        auto it = _roomToPage.find(roomNo);
        if (it == _roomToPage.end()) return -1;
        return it->second;
    }

    int16_t AddressScraper::getcurrentRoomNo(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return static_cast<int16_t>(H_(pageIndex, 0x01)); // BD-005: RoomId is at $01
    }

    // BD-005: $04=Up, $05=Down, $02=Left, $03=Right. No-neighbor sentinel = 0xFF.
    int16_t AddressScraper::getOverRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        const std::uint8_t v = H_(pageIndex, 0x04);
        return v == kNoNeighbor ? -1 : static_cast<int16_t>(v);
    }

    int16_t AddressScraper::getUnderRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        const std::uint8_t v = H_(pageIndex, 0x05);
        return v == kNoNeighbor ? -1 : static_cast<int16_t>(v);
    }

    int16_t AddressScraper::getLeftRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        const std::uint8_t v = H_(pageIndex, 0x02);
        return v == kNoNeighbor ? -1 : static_cast<int16_t>(v);
    }

    int16_t AddressScraper::getRightRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        const std::uint8_t v = H_(pageIndex, 0x03);
        return v == kNoNeighbor ? -1 : static_cast<int16_t>(v);
    }

    int16_t AddressScraper::getFrontRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        const std::uint8_t v = H_(pageIndex, 0x06);
        return v == kNoNeighbor ? -1 : static_cast<int16_t>(v);
    }

    int16_t AddressScraper::getBackRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        const std::uint8_t v = H_(pageIndex, 0x07);
        return v == kNoNeighbor ? -1 : static_cast<int16_t>(v);
    }

    // BD-005: $0A=ScrollUp, $0B=ScrollDown, $08=ScrollLeft, $09=ScrollRight.
    // Full byte per direction now (no nibble packing) -- raw RoomScrollType value (0..6).
    int16_t AddressScraper::getOverScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return static_cast<int16_t>(H_(pageIndex, 0x0A));
    }

    int16_t AddressScraper::getUnderScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return static_cast<int16_t>(H_(pageIndex, 0x0B));
    }

    int16_t AddressScraper::getLeftScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return static_cast<int16_t>(H_(pageIndex, 0x08));
    }

    int16_t AddressScraper::getRightScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return static_cast<int16_t>(H_(pageIndex, 0x09));
    }

    std::uint8_t AddressScraper::getZ(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return 0;
        return H_(pageIndex, 0x0C);
    }

    bool AddressScraper::isWater(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return false;
        return (H_(pageIndex, 0x0D) & PageFlagBits::Water) != 0;
    }

    bool AddressScraper::isWind(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return false;
        return (H_(pageIndex, 0x0D) & PageFlagBits::Wind) != 0;
    }

    // Is it possible to go over? (room exists && type is in the OLD scrollable set)
    // See TODO in AddressScraper.h -- pending Task B (ScrollKind redefinition).
    bool AddressScraper::isPossibleGoOver(std::size_t pageIndex) const
    {
        const auto r = getOverRoom(pageIndex);
        if (r < 0) return false;

        return isScrollableNibble_(static_cast<std::uint8_t>(getOverScrollType(pageIndex)));
    }

    bool AddressScraper::isPossibleGoUnder(std::size_t pageIndex) const
    {
        const auto r = getUnderRoom(pageIndex);
        if (r < 0) return false;

        return isScrollableNibble_(static_cast<std::uint8_t>(getUnderScrollType(pageIndex)));
    }

    bool AddressScraper::isPossibleGoLeft(std::size_t pageIndex) const
    {
        const auto r = getLeftRoom(pageIndex);
        if (r < 0) return false;

        return isScrollableNibble_(static_cast<std::uint8_t>(getLeftScrollType(pageIndex)));
    }

    bool AddressScraper::isPossibleGoRight(std::size_t pageIndex) const
    {
        const auto r = getRightRoom(pageIndex);
        if (r < 0) return false;

        return isScrollableNibble_(static_cast<std::uint8_t>(getRightScrollType(pageIndex)));
    }

    const std::uint8_t* AddressScraper::payloadPtr(std::size_t pageIndex) const noexcept
    {
        const std::size_t base = pageIndex * kPageSize + kPayloadOff;
        if (base + kPayloadSize <= _bin.size()) return _bin.data() + base;
        return nullptr;
    }

    void AddressScraper::buildPageIndexMap_()
    {
        _roomToPage.clear();
        const std::size_t pages = pageCount();
        for (std::size_t i = 0; i < pages; ++i)
        {
            // BD-005 magic check: $00=0xA5, $0F=0x5A. RoomId is at $01.
            if (H_(i, 0x00) == kPageHeaderMagicStart && H_(i, 0x0F) == kPageHeaderMagicEnd)
            {
                const std::uint8_t room = H_(i, 0x01);
                _roomToPage[room] = static_cast<int16_t>(i);
            }
        }
    }

    bool AddressScraper::inRange_(std::size_t pageIndex) const noexcept
    {
        return !_bin.empty() && (pageIndex * kPageSize + (kHeaderSize - 1)) < _bin.size();
    }

    std::uint8_t AddressScraper::H_(std::size_t pageIndex, std::size_t off) const noexcept
    {
        const std::size_t idx = pageIndex * kPageSize + off;
        if (idx < _bin.size()) return _bin[idx];
        return 0;
    }

    BGPageHeader AddressScraper::header_(std::size_t pageIndex) const noexcept
    {
        if (!inRange_(pageIndex)) return BGPageHeader{};
        const std::size_t base = pageIndex * kPageSize;
        return ParseBGPageHeader(_bin.data() + base);
    }
}