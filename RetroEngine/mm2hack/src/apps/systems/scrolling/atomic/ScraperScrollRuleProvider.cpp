#include "pch.h"

#include "ScraperScrollRuleProvider.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    using foundation::math::Vec2;

    using Kind = ScrollKind;

    static Kind ToKindOpt(const std::optional<Kind>& v) noexcept
    {
        return v.has_value() ? v.value() : Kind::None;
    }

    ScrollKind ScraperScrollRuleProvider::RightType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeRight(p) : std::optional<Kind>{});
    }

    ScrollKind ScraperScrollRuleProvider::LeftType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeLeft(p) : std::optional<Kind>{});
    }

    ScrollKind ScraperScrollRuleProvider::UpType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeUp(p) : std::optional<Kind>{});
    }

    ScrollKind ScraperScrollRuleProvider::DownType(std::size_t p) const
    {
        return ToKindOpt(_pageSrc ? _pageSrc->ScrollTypeDown(p) : std::optional<Kind>{});
    }

    int16_t ScraperScrollRuleProvider::RightRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborRight(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int16_t ScraperScrollRuleProvider::LeftRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborLeft(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int16_t ScraperScrollRuleProvider::UpRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborUp(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int16_t ScraperScrollRuleProvider::DownRoom(std::size_t p) const
    {
        if (!_pageSrc) return static_cast<int16_t>(-1);
        const auto opt = _pageSrc->NeighborDown(p);
        return opt ? static_cast<int16_t>(*opt) : static_cast<int16_t>(-1);
    }

    int ScraperScrollRuleProvider::ToPageIndex(uint8_t room) const
    {
        if (!_pageSrc) return -1;
        const auto opt = _pageSrc->RoomToPageIndex(room);
        return opt ? static_cast<int>(*opt) : -1;
    }

    Vec2 ScraperScrollRuleProvider::PageOriginPx(std::size_t page_index, int page_w, int page_h) const
    {
        ensureGridCacheBuilt_();

        const auto it = _grid.find(page_index);
        if (it == _grid.end())
        {
            // Not connected / unknown. Safe fallback.
            return Vec2{ 0.0, 0.0 };
        }

        const auto [gx, gy] = it->second;
        return Vec2{
            static_cast<double>(gx * page_w),
            static_cast<double>(gy * page_h)
        };
    }

    void ScraperScrollRuleProvider::tryAssignNeighbor_(std::size_t from, std::size_t to, int dx, int dy) const
    {
        const auto it = _grid.find(from);
        if (it == _grid.end()) { return; }

        const auto [fx, fy] = it->second;
        const auto nx = fx + dx;
        const auto ny = fy + dy;

        // First assignment wins (assumes consistent room graph)
        if (_grid.find(to) == _grid.end())
        {
            _grid.emplace(to, std::make_pair(nx, ny));
        }
    }

    void ScraperScrollRuleProvider::ensureGridCacheBuilt_() const
    {
        if (_gridBuilt) { return; }
        _gridBuilt = true;

        _grid.clear();
        _grid.emplace(0, std::make_pair(0, 0)); // page 0 -> origin

        std::deque<std::size_t> q;
        q.push_back(0);

        while (!q.empty())
        {
            const auto p = q.front();
            q.pop_front();

            const auto pushIfNew = [&](int16_t room, int dx, int dy)
                {
                    if (room < 0) { return; }
                    const int idx = ToPageIndex(static_cast<uint8_t>(room));
                    if (idx < 0) { return; }

                    const auto to = static_cast<std::size_t>(idx);
                    const bool existed = (_grid.find(to) != _grid.end());

                    tryAssignNeighbor_(p, to, dx, dy);

                    if (!existed)
                    {
                        q.push_back(to);
                    }
                };

            // Neighbor rooms (grid)
            pushIfNew(RightRoom(p), +1, 0);
            pushIfNew(LeftRoom(p), -1, 0);
            pushIfNew(UpRoom(p), 0, -1);
            pushIfNew(DownRoom(p), 0, +1);
        }
    }
}