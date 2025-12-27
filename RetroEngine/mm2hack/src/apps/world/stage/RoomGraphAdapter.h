//==============================================================================
// 
//  Project: mm2hack
//  RoomGraphAdapter.h
// 
//  Room graph adapter for rendering.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include "apps/resources/bg/AddressScraper.h"

namespace mm2hack::apps::world::stage
{
    using resources::bg::AddressScraper;

    // Adapter for room graph based on AddressScraper (Only entity class uses this)
    class RoomGraphAdapter
    {
    public:
        explicit RoomGraphAdapter(AddressScraper& s) : _s(s) {}

        // Convert roomNo <-> pageIndex
        int16_t PageIndexOf(uint8_t roomNo) const { return _s.getPageIndex(roomNo); }
        uint8_t RoomNoAtPage(int16_t pageIndex) const { return static_cast<uint8_t>(_s.getcurrentRoomNo(pageIndex)); }

        // Neighbors for given pageIndex
        std::optional<uint8_t> NeighborLeft(int16_t pageIndex)  const { return optRoom_(_s.getLeftRoom(pageIndex)); }
        std::optional<uint8_t> NeighborRight(int16_t pageIndex) const { return optRoom_(_s.getRightRoom(pageIndex)); }
        std::optional<uint8_t> NeighborUp(int16_t pageIndex)    const { return optRoom_(_s.getOverRoom(pageIndex)); }
        std::optional<uint8_t> NeighborDown(int16_t pageIndex)  const { return optRoom_(_s.getUnderRoom(pageIndex)); }

        // Check whether can go to the direction from given pageIndex
        bool CanGoLeft(int16_t pageIndex)  const { return _s.isPossibleGoLeft(pageIndex); }
        bool CanGoRight(int16_t pageIndex) const { return _s.isPossibleGoRight(pageIndex); }
        bool CanGoUp(int16_t pageIndex)    const { return _s.isPossibleGoOver(pageIndex); }
        bool CanGoDown(int16_t pageIndex)  const { return _s.isPossibleGoUnder(pageIndex); }

        // dir > 0: right/down, dir < 0: left/up, dir == 0: none
        [[nodiscard]] std::optional<std::size_t> AdjacentPageX(std::size_t pageIndex, int dir) const;
        // dir > 0: down, dir < 0: up, dir == 0: none
        [[nodiscard]] std::optional<std::size_t> AdjacentPageY(std::size_t pageIndex, int dir) const;

    private:
        // Convert -1 to nullopt, otherwise return uint8_t value
        static std::optional<uint8_t> optRoom_(int16_t v)
        {
            return (v >= 0) ? std::optional<uint8_t>(static_cast<uint8_t>(v)) : std::nullopt;
        }

    private:
        const std::wstring kClassName{ L"RoomGraphAdapter" };

        AddressScraper& _s;     // Reference to AddressScraper
    };
}