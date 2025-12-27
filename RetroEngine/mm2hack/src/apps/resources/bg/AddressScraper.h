//==============================================================================
// 
//  Project: mm2hack
//  AddressScraper.h
// 
//  Map address scraper for bg mapping binaries.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "apps/systems/scrolling/atomic/ScrollTypes.h"

namespace mm2hack::apps::resources::bg
{
    // Map address scraping utility
    class AddressScraper
    {
    public:
        // bin: 0x100 * N page structure. Each page [0x00..0x0F]=header, [0x10..0xFF]=tile 240B
        explicit AddressScraper(std::vector<std::uint8_t> bin);

        // Utility
        [[nodiscard]] const std::vector<std::uint8_t>& binary() const noexcept { return _bin; }
        [[nodiscard]] std::size_t pageCount() const noexcept { return _bin.size() / 0x100; }

        // Map: room_id(0x04) -> pageIndex
        [[nodiscard]] int16_t getPageIndex(std::size_t roomNo) const;  // Not found returns -1
        [[nodiscard]] int16_t getcurrentRoomNo(std::size_t pageIndex) const;

        // Return the room_id of the adjacent room (or -1 if not found). Specification: $05=Up, $06=Down, $07=Left, $08=Right
        [[nodiscard]] int16_t getOverRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getUnderRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getLeftRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getRightRoom(std::size_t pageIndex) const;

        // Scroll type (nibble): $09 upper=Up, lower=Down / $0A upper=Left, lower=Right
        [[nodiscard]] int16_t getOverScrollType(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getUnderScrollType(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getLeftScrollType(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getRightScrollType(std::size_t pageIndex) const;

        // Is scrollable? (true if room_id exists and nibble is {1,9,0x0A})
        [[nodiscard]] bool isPossibleGoOver(std::size_t pageIndex) const;
        [[nodiscard]] bool isPossibleGoUnder(std::size_t pageIndex) const;
        [[nodiscard]] bool isPossibleGoLeft(std::size_t pageIndex) const;
        [[nodiscard]] bool isPossibleGoRight(std::size_t pageIndex) const;

        // Binary in top of payload ([0x10..0xFF])
        [[nodiscard]] const std::uint8_t* payloadPtr(std::size_t pageIndex) const noexcept;

        std::vector<std::uint8_t> GetBin() const { return _bin; }

    private:
        void buildPageIndexMap_();
        [[nodiscard]] bool inRange_(std::size_t pageIndex) const noexcept;
        [[nodiscard]] std::uint8_t H_(std::size_t pageIndex, std::size_t off) const noexcept;   // Get header 1B
        [[nodiscard]] static bool isScrollableNibble_(std::uint8_t v) noexcept
        {
            using Scrl = mm2hack::apps::systems::scrolling::atomic::ScrollKind;
            auto const sv = static_cast<Scrl>(v);
            return sv == Scrl::FreeHorizontal || sv == Scrl::FollowObject || sv == Scrl::Free8Way;
        }

    private:
        const std::wstring kClassName{ L"AddressScraper" };

        std::vector<std::uint8_t> _bin;
        std::unordered_map<std::size_t, int16_t> _roomToPage; // room_id -> pageIndex
    };
}