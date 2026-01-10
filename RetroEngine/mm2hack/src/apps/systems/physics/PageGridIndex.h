//==============================================================================
// 
//  Project: mm2hack
//  PageGridIndex.h
// 
//  Page grid index for resolving page indices from world positions.
// 
//==============================================================================
#pragma once

#include <functional>
#include <optional>
#include <queue>
#include <unordered_map>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/stage/RoomGraphAdapter.h"

namespace mm2hack::apps::systems::physics
{
    using foundation::math::Vec2;

    struct GridPos
    {
        int gx{};
        int gy{};

        friend bool operator==(const GridPos& a, const GridPos& b) noexcept
        {
            return a.gx == b.gx && a.gy == b.gy;
        }
    };

    struct GridPosHash
    {
        std::size_t operator()(const GridPos& p) const noexcept
        {
            std::size_t h1 = std::hash<int>{}(p.gx);
            std::size_t h2 = std::hash<int>{}(p.gy);

            // Hash combine (unify all operations to std::size_t to avoid warnings)
            return h1 ^ (h2 + static_cast<std::size_t>(0x9e3779b97f4a7c15ULL) + (h1 << 6) + (h1 >> 2));
        }
    };

    // Build a grid embedding for a page graph so we can resolve pageIndex from world position
    class PageGridIndex final
    {
        using AdjacentPolicy = world::stage::AdjacentPolicy;

    public:
        PageGridIndex(double pageW, double pageH) noexcept
            : _pageW(pageW), _pageH(pageH)
        {
        }

        template <class GraphAdapter>
        void Build(GraphAdapter& src, int startPageIndex)
        {
            _pageToGrid.clear();
            _gridToPage.clear();

            std::queue<int> q;
            _pageToGrid.emplace(startPageIndex, GridPos{ 0, 0 });
            _gridToPage.emplace(GridPos{ 0, 0 }, startPageIndex);
            q.push(startPageIndex);

            while (!q.empty())
            {
                const int cur = q.front();
                q.pop();

                const auto& curGrid = _pageToGrid.at(cur);

                // Right / Left
                expandX_(src, cur, curGrid, +1, q);
                expandX_(src, cur, curGrid, -1, q);

                // Down / Up  (y+ is down in screen coords; still fine as "grid" concept)
                expandY_(src, cur, curGrid, +1, q);
                expandY_(src, cur, curGrid, -1, q);
            }
        }

        static int FloorDiv(double v, double d) noexcept
        {
            // floor(v / d) for negatives too
            const double q = v / d;
            const int i = static_cast<int>(q);
            return (q < 0.0 && q != static_cast<double>(i)) ? (i - 1) : i;
        }

        // Resolve page index from world center position
        std::optional<int> ResolvePageIndexFromWorldPos(const Vec2& worldCenter) const noexcept
        {
            const int gx = FloorDiv(worldCenter.x, _pageW);
            const int gy = FloorDiv(worldCenter.y, _pageH);

            const auto it = _gridToPage.find(GridPos{ gx, gy });
            if (it == _gridToPage.end()) return std::nullopt;
            return it->second;
        }

        // Get world origin position of a page index
        std::optional<Vec2> GetPageWorldOrigin(int pageIndex) const noexcept
        {
            const auto it = _pageToGrid.find(pageIndex);
            if (it == _pageToGrid.end()) return std::nullopt;
            const GridPos& gp = it->second;
            return Vec2{ static_cast<double>(gp.gx) * _pageW,
                         static_cast<double>(gp.gy) * _pageH };
        }

        // Convert world position to local (page-relative) position [0..pageSize)
        [[nodiscard]] double ToLocalPos(double worldPos, int pageSize) const noexcept
        {
            const int g = FloorDiv(worldPos, static_cast<double>(pageSize));
            return worldPos - static_cast<double>(g) * pageSize;
        }

    private:
        template <class GraphAdapter>
        void expandX_(GraphAdapter& src, int curPage, GridPos curGrid, int dir, std::queue<int>& q)
        {
            const auto next = src.AdjacentPageX(curPage, dir, AdjacentPolicy::AnyConnection); // optional<int>
            if (!next) return;

            const GridPos ng{ curGrid.gx + dir, curGrid.gy };
            tryInsert_(static_cast<int>(*next), ng, q);
        }

        template <class GraphAdapter>
        void expandY_(GraphAdapter& src, int curPage, GridPos curGrid, int dir, std::queue<int>& q)
        {
            const auto next = src.AdjacentPageY(curPage, dir, AdjacentPolicy::AnyConnection); // optional<int>
            if (!next) return;

            const GridPos ng{ curGrid.gx, curGrid.gy + dir };
            tryInsert_(static_cast<int>(*next), ng, q);
        }

        void tryInsert_(int page, GridPos grid, std::queue<int>& q)
        {
            if (_pageToGrid.contains(page)) return;              // already assigned
            if (_gridToPage.contains(grid)) return;              // collision in embedding (rare but possible)

            _pageToGrid.emplace(page, grid);
            _gridToPage.emplace(grid, page);
            q.push(page);
        }

        double _pageW{};
        double _pageH{};

        std::unordered_map<int, GridPos> _pageToGrid;
        std::unordered_map<GridPos, int, GridPosHash> _gridToPage;
    };
}