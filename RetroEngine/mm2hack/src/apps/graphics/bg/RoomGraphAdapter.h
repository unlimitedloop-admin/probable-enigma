//==============================================================================
// 
//  Project: mm2hack
//  RoomGraphAdapter.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include "AddressScraper.h"

namespace mm2hack::apps::graphics::bg
{
    class RoomGraphAdapter
    {
    public:
        explicit RoomGraphAdapter(AddressScraper& s) : _s(s) {}

        // 現在roomNoからページindexを得る
        int16_t PageIndexOf(uint8_t roomNo) const { return _s.getPageIndex(roomNo); }
        uint8_t RoomNoAtPage(int16_t pageIndex) const { return static_cast<uint8_t>(_s.getcurrentRoomNo(pageIndex)); }

        std::optional<uint8_t> NeighborLeft(int16_t pageIndex)  const { return opt_(_s.getLeftRoom(pageIndex)); }
        std::optional<uint8_t> NeighborRight(int16_t pageIndex)  const { return opt_(_s.getRightRoom(pageIndex)); }
        std::optional<uint8_t> NeighborUp(int16_t pageIndex)  const { return opt_(_s.getOverRoom(pageIndex)); }
        std::optional<uint8_t> NeighborDown(int16_t pageIndex)  const { return opt_(_s.getUnderRoom(pageIndex)); }

        // 可否判定（スクロール種別を内部で見ている前提）
        bool CanGoLeft(int16_t pageIndex) const { return _s.isPossibleGoLeft(pageIndex); }
        bool CanGoRight(int16_t pageIndex) const { return _s.isPossibleGoRight(pageIndex); }
        bool CanGoUp(int16_t pageIndex) const { return _s.isPossibleGoOver(pageIndex); }
        bool CanGoDown(int16_t pageIndex) const { return _s.isPossibleGoUnder(pageIndex); }

    private:
        static std::optional<uint8_t> opt_(int16_t v) { return v >= 0 ? std::optional<uint8_t>(static_cast<uint8_t>(v)) : std::nullopt; }

    private:
        AddressScraper& _s;
    };
}