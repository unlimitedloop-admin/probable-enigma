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

    // TODO: AdjacentPageX/Y は .cpp(新規作成) 側に実装を切り出す
    class RoomGraphAdapter
    {
    public:
        explicit RoomGraphAdapter(AddressScraper& s) : _s(s) {}

        int16_t PageIndexOf(uint8_t roomNo) const { return _s.getPageIndex(roomNo); }
        uint8_t RoomNoAtPage(int16_t pageIndex) const { return static_cast<uint8_t>(_s.getcurrentRoomNo(pageIndex)); }

        std::optional<uint8_t> NeighborLeft(int16_t pageIndex)  const { return optRoom_(_s.getLeftRoom(pageIndex)); }
        std::optional<uint8_t> NeighborRight(int16_t pageIndex) const { return optRoom_(_s.getRightRoom(pageIndex)); }
        std::optional<uint8_t> NeighborUp(int16_t pageIndex)    const { return optRoom_(_s.getOverRoom(pageIndex)); }
        std::optional<uint8_t> NeighborDown(int16_t pageIndex)  const { return optRoom_(_s.getUnderRoom(pageIndex)); }

        bool CanGoLeft(int16_t pageIndex)  const { return _s.isPossibleGoLeft(pageIndex); }
        bool CanGoRight(int16_t pageIndex) const { return _s.isPossibleGoRight(pageIndex); }
        bool CanGoUp(int16_t pageIndex)    const { return _s.isPossibleGoOver(pageIndex); }
        bool CanGoDown(int16_t pageIndex)  const { return _s.isPossibleGoUnder(pageIndex); }

        // ★追加：dir は -1(Left/Up) / +1(Right/Down)
        [[nodiscard]] std::optional<std::size_t> AdjacentPageX(std::size_t pageIndex, int dir) const
        {
            if (dir == 0) return std::nullopt;

            const int16_t pi = static_cast<int16_t>(pageIndex);
            const bool can = (dir > 0) ? CanGoRight(pi) : CanGoLeft(pi);
            if (!can) return std::nullopt;

            const auto nr = (dir > 0) ? NeighborRight(pi) : NeighborLeft(pi);
            if (!nr) return std::nullopt;

            const int16_t next = PageIndexOf(*nr);
            return (next >= 0) ? std::optional<std::size_t>(static_cast<std::size_t>(next)) : std::nullopt;
        }

        [[nodiscard]] std::optional<std::size_t> AdjacentPageY(std::size_t pageIndex, int dir) const
        {
            if (dir == 0) return std::nullopt;

            const int16_t pi = static_cast<int16_t>(pageIndex);
            const bool can = (dir > 0) ? CanGoDown(pi) : CanGoUp(pi);
            if (!can) return std::nullopt;

            const auto nr = (dir > 0) ? NeighborDown(pi) : NeighborUp(pi);
            if (!nr) return std::nullopt;

            const int16_t next = PageIndexOf(*nr);
            return (next >= 0) ? std::optional<std::size_t>(static_cast<std::size_t>(next)) : std::nullopt;
        }

    private:
        static std::optional<uint8_t> optRoom_(int16_t v)
        {
            return (v >= 0) ? std::optional<uint8_t>(static_cast<uint8_t>(v)) : std::nullopt;
        }

        const std::wstring kClassName{ L"RoomGraphAdapter" };
        AddressScraper& _s;
    };
}