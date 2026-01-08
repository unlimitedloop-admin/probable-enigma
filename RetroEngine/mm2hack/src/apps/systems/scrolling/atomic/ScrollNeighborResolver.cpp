#include "pch.h"

#include "ScrollNeighborResolver.h"

#include "IScrollRuleProvider.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    int ScrollNeighborResolver::ResolveNextIndexX(const std::size_t page_index, const int dir) const
    {
        if (dir == 0)
        {
            return -1;
        }

        const ScrollKind kind = (dir > 0) ? _rules.RightType(page_index) : _rules.LeftType(page_index);
        const int16_t room = (dir > 0) ? _rules.RightRoom(page_index) : _rules.LeftRoom(page_index);

        if (!IsAllowedFree(kind) || room < 0)
        {
            return -1;
        }

        const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
        return (idx >= 0) ? idx : -1;
    }

    int ScrollNeighborResolver::ResolveNextIndexY(const std::size_t page_index, const int dir) const
    {
        if (dir == 0)
        {
            return -1;
        }

        const ScrollKind kind = (dir > 0) ? _rules.DownType(page_index) : _rules.UpType(page_index);
        const int16_t room = (dir > 0) ? _rules.DownRoom(page_index) : _rules.UpRoom(page_index);

        if (!IsAllowedFree(kind) || room < 0)
        {
            return -1;
        }

        const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
        return (idx >= 0) ? idx : -1;
    }

    std::optional<std::size_t> ScrollNeighborResolver::ResolveFixedNeighbor(const PageScroll::Dir dir, const std::size_t from) const
    {
        switch (dir)
        {
        case PageScroll::Dir::Right:
            if (!IsFixedScroll(_rules.RightType(from))) return std::nullopt;
            return roomToIndex_(_rules, _rules.RightRoom(from));
        case PageScroll::Dir::Left:
            if (!IsFixedScroll(_rules.LeftType(from))) return std::nullopt;
            return roomToIndex_(_rules, _rules.LeftRoom(from));
        case PageScroll::Dir::Down:
            if (!IsFixedScroll(_rules.DownType(from))) return std::nullopt;
            return roomToIndex_(_rules, _rules.DownRoom(from));
        case PageScroll::Dir::Up:
            if (!IsFixedScroll(_rules.UpType(from))) return std::nullopt;
            return roomToIndex_(_rules, _rules.UpRoom(from));
        default:
            return std::nullopt;
        }
    }

    std::optional<std::size_t> ScrollNeighborResolver::roomToIndex_(const IScrollRuleProvider& rules, const int16_t room) noexcept
    {
        if (room < 0)
        {
            return std::nullopt;
        }

        const int idx = rules.ToPageIndex(static_cast<uint8_t>(room));
        if (idx < 0)
        {
            return std::nullopt;
        }

        return static_cast<std::size_t>(idx);
    }
}
