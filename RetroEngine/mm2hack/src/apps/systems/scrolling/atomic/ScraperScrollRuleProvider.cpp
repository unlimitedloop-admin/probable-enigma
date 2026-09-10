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

    void ScraperScrollRuleProvider::tryAssignNeighbor_(std::unordered_map<std::size_t, std::pair<int, int>>& grid, std::size_t from, std::size_t to, int dx, int dy) const
    {
        auto itFrom = grid.find(from);
        if (itFrom == grid.end()) return;

        const auto [fx, fy] = itFrom->second;
        const std::pair<int, int> want{ fx + dx, fy + dy };

        auto itTo = grid.find(to);
        if (itTo == grid.end())
        {
            // First time: assign.
            grid.emplace(to, want);
            return;
        }

        // Already assigned: verify consistency (DO NOT overwrite).
        const auto& cur = itTo->second;
        if (cur != want)
        {
            // Here is where a loop/inconsistent rules show up.
            // Keep the existing one to avoid breaking the whole world mapping.
            // Optional: log for debugging.
            // e.g. Log("[ScrollGrid] conflict: to=%zu cur=(%d,%d) want=(%d,%d) from=%zu",
            //          to, cur.first, cur.second, want.first, want.second, from);
        }
    }

    void ScraperScrollRuleProvider::ensureGridCacheBuilt_() const
    {
        if (_gridBuilt) return;

        decltype(_grid) tmp;
        tmp.clear();

        // Page 0 is the XY world origin anchor.
        tmp.emplace(0, std::make_pair(0, 0));

        std::deque<std::size_t> queue;
        queue.push_back(0);

        while (!queue.empty())
        {
            const std::size_t page_index = queue.front();
            queue.pop_front();

            const auto pushIfNew =
                [&](int16_t neighbor_page_index, int dx, int dy)
                {
                    if (neighbor_page_index < 0) return;

                    const auto destination = static_cast<std::size_t>(neighbor_page_index);
                    const bool existed = tmp.find(destination) != tmp.end();

                    tryAssignNeighbor_(tmp, page_index, destination, dx, dy);

                    if (!existed)
                    {
                        queue.push_back(destination);
                    }
                };

            pushIfNew(RightRoom(page_index), +1, 0);
            pushIfNew(LeftRoom(page_index), -1, 0);
            pushIfNew(UpRoom(page_index), 0, -1);
            pushIfNew(DownRoom(page_index), 0, +1);
        }

        _grid.swap(tmp);
        _gridBuilt = true;
    }
}