#include "pch.h"

#include "RoomGraphAdapter.h"

namespace mm2hack::apps::world::stage
{
    // dir > 0: right/down, dir < 0: left/up, dir == 0: none
    [[nodiscard]] std::optional<std::size_t> RoomGraphAdapter::AdjacentPageX(std::size_t pageIndex, int dir) const
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

    // dir > 0: down, dir < 0: up, dir == 0: none
    [[nodiscard]] std::optional<std::size_t> RoomGraphAdapter::AdjacentPageY(std::size_t pageIndex, int dir) const
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
}