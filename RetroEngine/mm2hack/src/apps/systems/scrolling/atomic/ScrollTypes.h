//==============================================================================
// 
//  Project: mm2hack
//  ScrollTypes.h
// 
//  Optimized scrolling type definitions.
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::systems::scrolling::atomic
{
    enum class ScrollKind : int
    {
        None = 0x00,
        FreeHorizontal = 0x01,  // Side free scroll
        FixedPage = 0x02,       // Fixed page (animation)
        FollowObject = 0x09,    // Object follow
        Free8Way = 0x0A         // 8-direction scroll
    };

    struct PageScroll
    {
        enum class Dir { None, Right, Left, Down, Up };

        bool active{ false };
        Dir dir{ Dir::None };
        double progress{ 0.0 };
        double speed{ 4.0 };

        std::size_t from_index{ 0 };
        std::size_t to_index{ 0 };
    };

    struct FixedScrollRequest
    {
        PageScroll::Dir dir{ PageScroll::Dir::None };
        double carryTotalPx{ 0.0 };     // Total distance to move the player during the whole fixed-scroll animation (world px).
    };

    // Helpers
    inline constexpr bool IsHorizontal(PageScroll::Dir d) noexcept
    {
        return d == PageScroll::Dir::Left || d == PageScroll::Dir::Right;
    }

    inline constexpr bool IsVertical(PageScroll::Dir d) noexcept
    {
        return d == PageScroll::Dir::Up || d == PageScroll::Dir::Down;
    }

    inline constexpr int DirSignX(PageScroll::Dir d) noexcept
    {
        if (d == PageScroll::Dir::Right) return +1;
        if (d == PageScroll::Dir::Left)  return -1;
        return 0;
    }

    inline constexpr int DirSignY(PageScroll::Dir d) noexcept
    {
        if (d == PageScroll::Dir::Down) return +1;
        if (d == PageScroll::Dir::Up)   return -1;
        return 0;
    }

    inline constexpr double ClampNonNeg(double v) noexcept
    {
        return (v < 0.0) ? 0.0 : v;
    }

    // Check if the scroll kind allows free movement
    inline constexpr bool IsAllowedFree(ScrollKind k) noexcept
    {
        return (k == ScrollKind::FreeHorizontal) || (k == ScrollKind::FollowObject) || (k == ScrollKind::Free8Way);
    }

    inline constexpr bool IsFixedScroll(ScrollKind k) noexcept
    {
        return (k == ScrollKind::FixedPage);
    }
}