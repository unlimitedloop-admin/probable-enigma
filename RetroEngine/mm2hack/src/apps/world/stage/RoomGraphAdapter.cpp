#include "pch.h"

#include "RoomGraphAdapter.h"

namespace mm2hack::apps::world::stage
{
    // dir > 0: right, dir < 0: left, dir == 0: none
    std::optional<std::size_t> RoomGraphAdapter::AdjacentPageX(std::size_t pageIndex, int dir, AdjacentPolicy policy) const
    {
        if (dir == 0) return std::nullopt;

        const int16_t pi = static_cast<int16_t>(pageIndex);

        // Decide if the edge is connected.
        auto hasNeighbor = [&](int16_t p) -> bool
            {
                if (dir > 0)
                {
                    const auto r = NeighborRight(p);   // returns optional<roomNo> etc
                    if (!r) return false;

                    if (policy == AdjacentPolicy::FreeOnly)
                    {
                        return _s.isPossibleGoRight(static_cast<std::size_t>(p));
                    }
                    return true; // AnyConnection: neighbor exists => connected
                }
                else
                {
                    const auto r = NeighborLeft(p);
                    if (!r) return false;

                    if (policy == AdjacentPolicy::FreeOnly)
                    {
                        return _s.isPossibleGoLeft(static_cast<std::size_t>(p));
                    }
                    return true;
                }
            };

        if (!hasNeighbor(pi)) return std::nullopt;

        const auto nr = (dir > 0) ? NeighborRight(pi) : NeighborLeft(pi);
        if (!nr) return std::nullopt;

        const int16_t next = PageIndexOf(*nr);
        return (next >= 0) ? std::optional<std::size_t>(static_cast<std::size_t>(next)) : std::nullopt;
    }

    // dir > 0: down, dir < 0: up, dir == 0: none
    [[nodiscard]] std::optional<std::size_t> RoomGraphAdapter::AdjacentPageY(std::size_t pageIndex, int dir, AdjacentPolicy policy) const
    {
        if (dir == 0) return std::nullopt;
        const int16_t pi = static_cast<int16_t>(pageIndex);
        // Decide if the edge is connected.
        auto hasNeighbor = [&](int16_t p) -> bool
            {
                if (dir > 0)
                {
                    const auto r = NeighborDown(p);   // returns optional<roomNo> etc
                    if (!r) return false;
                    if (policy == AdjacentPolicy::FreeOnly)
                    {
                        return _s.isPossibleGoUnder(static_cast<std::size_t>(p));
                    }
                    return true; // AnyConnection: neighbor exists => connected
                }
                else
                {
                    const auto r = NeighborUp(p);
                    if (!r) return false;
                    if (policy == AdjacentPolicy::FreeOnly)
                    {
                        return _s.isPossibleGoOver(static_cast<std::size_t>(p));
                    }
                    return true;
                }
            };
        if (!hasNeighbor(pi)) return std::nullopt;
        const auto nr = (dir > 0) ? NeighborDown(pi) : NeighborUp(pi);
        if (!nr) return std::nullopt;
        const int16_t next = PageIndexOf(*nr);
        return (next >= 0) ? std::optional<std::size_t>(static_cast<std::size_t>(next)) : std::nullopt;
    }
}