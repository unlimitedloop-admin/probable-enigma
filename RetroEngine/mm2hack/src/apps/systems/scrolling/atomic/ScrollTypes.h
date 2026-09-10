//==============================================================================
//
//  Project: mm2hack
//  ScrollTypes.h
//
//  Optimized scrolling type definitions.
//
//==============================================================================
#pragma once

#include <cstdint>

#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // Defines the runtime scrolling behavior.
    // Values intentionally match resources::bg::RoomScrollType (BD-005).
    enum class ScrollKind : std::uint8_t
    {
        None         = 0x00,
        Free         = 0x01,
        Page         = 0x02,
        Auto         = 0x03,
        ObjectFollow = 0x04,
        EventDriven  = 0x05,
        Loop         = 0x06
    };

    struct PageScroll
    {
        enum class Dir
        {
            None,
            Right,
            Left,
            Down,
            Up
        };

        bool active{ false };
        Dir dir{ Dir::None };
        double progress{ 0.0 };
        double speed{ 4.0 };

        std::size_t from_index{ 0 };
        std::size_t to_index{ 0 };
    };

    struct FixedScrollRequest
    {
        bool available{ false };
        PageScroll::Dir dir{ PageScroll::Dir::None };
        double carryTotalPx{ 0.0 };
    };

    struct ScrollEffect
    {
        bool fixedActive{ false };
        foundation::math::Vec2 playerDelta{};
    };

    enum class ScrollMode : int
    {
        None = 0x00,
        PlayerFollow,
        FixedPage,
        AutoScroll,
        TargetFollow
    };

    struct ViewBounds
    {
        double leftX{};
        double rightX{};
        double topY{};
        double bottomY{};
    };

    struct FixedScrollMeasure
    {
        using Vec2 = foundation::math::Vec2;

        ViewBounds fromBounds{};
        Vec2 pageOriginPx{};
    };

    [[nodiscard]] inline constexpr bool IsHorizontal(PageScroll::Dir dir) noexcept
    {
        return dir == PageScroll::Dir::Left || dir == PageScroll::Dir::Right;
    }

    [[nodiscard]] inline constexpr bool IsVertical(PageScroll::Dir dir) noexcept
    {
        return dir == PageScroll::Dir::Up || dir == PageScroll::Dir::Down;
    }

    [[nodiscard]] inline constexpr int DirSignX(PageScroll::Dir dir) noexcept
    {
        if (dir == PageScroll::Dir::Right)
        {
            return +1;
        }

        if (dir == PageScroll::Dir::Left)
        {
            return -1;
        }

        return 0;
    }

    [[nodiscard]] inline constexpr int DirSignY(PageScroll::Dir dir) noexcept
    {
        if (dir == PageScroll::Dir::Down)
        {
            return +1;
        }

        if (dir == PageScroll::Dir::Up)
        {
            return -1;
        }

        return 0;
    }

    [[nodiscard]] inline constexpr double ClampNonNeg(double value) noexcept
    {
        return value < 0.0 ? 0.0 : value;
    }

    [[nodiscard]] inline constexpr bool IsAllowedFree(ScrollKind kind) noexcept
    {
        return kind == ScrollKind::Free;
    }

    [[nodiscard]] inline constexpr bool IsFixedScroll(ScrollKind kind) noexcept
    {
        return kind == ScrollKind::Page;
    }
}
