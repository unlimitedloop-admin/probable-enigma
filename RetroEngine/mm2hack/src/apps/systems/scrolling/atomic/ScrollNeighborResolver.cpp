#include "pch.h"

#include "ScrollNeighborResolver.h"

#include "IScrollRuleProvider.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    int ScrollNeighborResolver::ResolveNextIndexX(const std::size_t page_index, const int dir) const
    {
        if (dir == 0) return -1;

        const ScrollKind kind =
            dir > 0
            ? _rules.RightType(page_index)
            : _rules.LeftType(page_index);

        const int16_t next_page_index =
            dir > 0
            ? _rules.RightRoom(page_index)
            : _rules.LeftRoom(page_index);

        if (!IsAllowedFree(kind) || next_page_index < 0) return -1;

        return static_cast<int>(next_page_index);
    }

    int ScrollNeighborResolver::ResolveNextIndexY(const std::size_t page_index, const int dir) const
    {
        if (dir == 0) return -1;

        const ScrollKind kind =
            dir > 0
            ? _rules.DownType(page_index)
            : _rules.UpType(page_index);

        const int16_t next_page_index =
            dir > 0
            ? _rules.DownRoom(page_index)
            : _rules.UpRoom(page_index);

        if (!IsAllowedFree(kind) || next_page_index < 0) return -1;

        return static_cast<int>(next_page_index);
    }

    std::optional<std::size_t> ScrollNeighborResolver::ResolveFixedNeighbor(const PageScroll::Dir dir, const std::size_t from) const
    {
        int16_t next_page_index = -1;

        switch (dir)
        {
        case PageScroll::Dir::Right:
            if (!IsFixedScroll(_rules.RightType(from))) return std::nullopt;
            next_page_index = _rules.RightRoom(from);
            break;

        case PageScroll::Dir::Left:
            if (!IsFixedScroll(_rules.LeftType(from))) return std::nullopt;
            next_page_index = _rules.LeftRoom(from);
            break;

        case PageScroll::Dir::Down:
            if (!IsFixedScroll(_rules.DownType(from))) return std::nullopt;
            next_page_index = _rules.DownRoom(from);
            break;

        case PageScroll::Dir::Up:
            if (!IsFixedScroll(_rules.UpType(from))) return std::nullopt;
            next_page_index = _rules.UpRoom(from);
            break;

        default:
            return std::nullopt;
        }

        if (next_page_index < 0) return std::nullopt;

        return static_cast<std::size_t>(next_page_index);
    }
}
