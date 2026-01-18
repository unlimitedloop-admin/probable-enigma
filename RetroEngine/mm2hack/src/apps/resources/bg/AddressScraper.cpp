#include "pch.h"

#include "AddressScraper.h"

namespace
{
    using namespace mm2hack::config;

    constexpr std::size_t kPageSize = SystemConfig::kMapBinaryUnitPageSize;
    constexpr std::size_t kHeaderSize = SystemConfig::kMapBinaryHeaderSize; // [0x00..0x0F] inclusive
    constexpr std::size_t kPayloadOff = SystemConfig::kMapBinaryHeaderSize;
    constexpr std::size_t kPayloadEnd = kPageSize - kHeaderSize;            // [0x10..0xFF] inclusive
    constexpr std::size_t kPayloadSize = kPayloadEnd;                       // 240B
    inline std::uint8_t hi4(std::uint8_t v) { return static_cast<std::uint8_t>((v >> 4) & 0x0F); }
    inline std::uint8_t lo4(std::uint8_t v) { return static_cast<std::uint8_t>(v & 0x0F); }
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
        return static_cast<int16_t>(H_(pageIndex, 0x04));
    }

    // Return the room_id of the adjacent room (or -1 if not found). Specification: $05=Up, $06=Down, $07=Left, $08=Right
    int16_t AddressScraper::getOverRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex))
        {
            return -1;
        }
        return H_(pageIndex, 0x05) == 0x00 ? -1 : static_cast<int16_t>(H_(pageIndex, 0x05));
    }

    int16_t AddressScraper::getUnderRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex))
        {
            return -1;
        }
        return H_(pageIndex, 0x06) == 0x00 ? -1 : static_cast<int16_t>(H_(pageIndex, 0x06));
    }

    int16_t AddressScraper::getLeftRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex))
        {
            return -1;
        }
        return H_(pageIndex, 0x07) == 0x00 ? -1 : static_cast<int16_t>(H_(pageIndex, 0x07));
    }

    int16_t AddressScraper::getRightRoom(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex))
        {
            return -1;
        }
        return H_(pageIndex, 0x08) == 0x00 ? -1 : static_cast<int16_t>(H_(pageIndex, 0x08));
    }

    // Scroll type (nibble)
    int16_t AddressScraper::getOverScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return hi4(H_(pageIndex, 0x09));
    }

    int16_t AddressScraper::getUnderScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return lo4(H_(pageIndex, 0x09));
    }

    int16_t AddressScraper::getLeftScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return hi4(H_(pageIndex, 0x0A));
    }

    int16_t AddressScraper::getRightScrollType(std::size_t pageIndex) const
    {
        if (!inRange_(pageIndex)) return -1;
        return lo4(H_(pageIndex, 0x0A));
    }

    // Is it possible to go over? (room exists && type is 1 / 9 / 0x0A)
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
            // NOTE: $00=01, $01=FF, $0F=FF (light check, strict verification is separate)
            if (H_(i, 0x00) == 0x01 && H_(i, 0x01) == 0xFF && H_(i, 0x0F) == 0xFF)
            {
                const std::uint8_t room = H_(i, 0x04);
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
}