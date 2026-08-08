//==============================================================================
// 
//  Project: mm2hack
//  AddressScraper.h
// 
//  Map address scraper for bg mapping binaries.
// 
//  [MIGRATION NOTE]
//  This class now decodes pages using the BD-005 page header layout
//  (see BGPageHeader.h). It replaces the old ad-hoc offset table.
// 
//  Return types (int16_t) are kept unchanged so downstream callers
//  (MapPageCache, etc.) do not need to change today. The values
//  returned by getXScrollType() now come directly from the new
//  RoomScrollType byte range (0..6) instead of the old nibble-packed
//  encoding. Reconciling this with atomic::ScrollKind (which still
//  only names 0x00/0x01/0x02/0x09/0x0A) is tracked separately -- see
//  the TODO on isPossibleGoX below.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "BGPageHeader.h"

namespace mm2hack::apps::resources::bg
{
    // Map address scraping utility
    class AddressScraper
    {
    public:
        // bin: 0x100 * N page structure. Each page [0x00..0x0F]=header (BGPageHeader), [0x10..0xFF]=tile 240B
        explicit AddressScraper(std::vector<std::uint8_t> bin);

        // Utility
        [[nodiscard]] const std::vector<std::uint8_t>& binary() const noexcept { return _bin; }
        [[nodiscard]] std::size_t pageCount() const noexcept { return _bin.size() / 0x100; }

        // Map: room_id ($01) -> pageIndex
        [[nodiscard]] int16_t getPageIndex(std::size_t roomNo) const;  // Not found returns -1
        [[nodiscard]] int16_t getcurrentRoomNo(std::size_t pageIndex) const;

        // Return the room_id of the adjacent room (or -1 if not found).
        // BD-005: $02=Left, $03=Right, $04=Up, $05=Down (no-neighbor sentinel = 0xFF)
        [[nodiscard]] int16_t getOverRoom(std::size_t pageIndex) const;   // Up
        [[nodiscard]] int16_t getUnderRoom(std::size_t pageIndex) const;  // Down
        [[nodiscard]] int16_t getLeftRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getRightRoom(std::size_t pageIndex) const;

        // Front/Back neighbors (Z-axis). Not exposed via IMapPageSource yet;
        // kept here so callers that need Z-layer data have a source once
        // that interface is extended.
        [[nodiscard]] int16_t getFrontRoom(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getBackRoom(std::size_t pageIndex) const;

        // Scroll type: full byte per direction (RoomScrollType raw value, 0..6).
        // BD-005: $08=ScrollLeft, $09=ScrollRight, $0A=ScrollUp, $0B=ScrollDown
        [[nodiscard]] int16_t getOverScrollType(std::size_t pageIndex) const;   // Up
        [[nodiscard]] int16_t getUnderScrollType(std::size_t pageIndex) const;  // Down
        [[nodiscard]] int16_t getLeftScrollType(std::size_t pageIndex) const;
        [[nodiscard]] int16_t getRightScrollType(std::size_t pageIndex) const;

        // Z / Flags (new in BD-005; not read by anything yet, exposed for future use)
        [[nodiscard]] std::uint8_t getZ(std::size_t pageIndex) const;
        [[nodiscard]] bool isWater(std::size_t pageIndex) const;
        [[nodiscard]] bool isWind(std::size_t pageIndex) const;

        // TODO(Task B): these still gate on the OLD ScrollKind scrollable set
        // {FreeHorizontal, FollowObject, Free8Way}. Now that getXScrollType()
        // returns raw RoomScrollType values (0..6), this check is stale --
        // e.g. PageEdge(2)/ObjectFollow(4)/Loop(6) will currently be judged
        // "not possible" even though they conceptually allow movement.
        // Left as-is until ScrollKind is redefined (Task B) so behavior here
        // doesn't silently change ahead of that migration.
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
        [[nodiscard]] BGPageHeader header_(std::size_t pageIndex) const noexcept;               // Parse full header
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