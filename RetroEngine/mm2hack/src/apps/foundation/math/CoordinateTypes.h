//==============================================================================
// 
//  Project: mm2hack
//  CoordinateTypes.h
// 
//  A unified structure for handling coordinate information.
// 
//==============================================================================
#pragma once

#include <cmath>

namespace mm2hack::apps::foundation::math
{
    constexpr double kEps = 1e-9;

    // Vector 2D (direction and magnitude), Point - Point, Point + Vector = Point
    struct Vec2
    {
        double x{ 0 }, y{ 0 };

        constexpr Vec2() = default;
        constexpr Vec2(double x_, double y_) noexcept : x(x_), y(y_) {}

        [[nodiscard]] constexpr Vec2 operator+(const Vec2& o) const noexcept { return { x + o.x, y + o.y }; }
        [[nodiscard]] constexpr Vec2 operator-(const Vec2& o) const noexcept { return { x - o.x, y - o.y }; }
        [[nodiscard]] constexpr Vec2 operator*(double s) const noexcept { return { x * s, y * s }; }
        [[nodiscard]] constexpr Vec2 operator/(double s) const noexcept { return { x / s, y / s }; }

        constexpr Vec2& operator+=(const Vec2& o) noexcept { x += o.x; y += o.y; return *this; }
        constexpr Vec2& operator-=(const Vec2& o) noexcept { x -= o.x; y -= o.y; return *this; }
        constexpr Vec2& operator*=(double s) noexcept { x *= s; y *= s; return *this; }
        constexpr Vec2& operator/=(double s) noexcept { x /= s; y /= s; return *this; }

        [[nodiscard]] double length() const noexcept { return std::sqrt(x * x + y * y); }
        [[nodiscard]] double lengthSq() const noexcept { return x * x + y * y; }
        [[nodiscard]] Vec2 normalized(double eps = kEps) const noexcept
        {
            const auto len = length();
            return (len <= eps) ? Vec2{ 0,0 } : Vec2{ x / len, y / len };
        }
        static constexpr Vec2 Zero() noexcept { return {}; }
    };
    inline constexpr Vec2 operator*(double s, const Vec2& v) noexcept { return { v.x * s, v.y * s }; }

    // 2D Point (position). Point + Vector = Point / Point - Point = Vector
    struct Point2
    {
        double x{ 0 }, y{ 0 };

        constexpr Point2() = default;
        constexpr Point2(double x_, double y_) noexcept : x(x_), y(y_) {}

        [[nodiscard]] constexpr Point2 operator+(const Vec2& v) const noexcept { return { x + v.x, y + v.y }; }
        [[nodiscard]] constexpr Point2 operator-(const Vec2& v) const noexcept { return { x - v.x, y - v.y }; }
        [[nodiscard]] constexpr Vec2   operator-(const Point2& p) const noexcept { return { x - p.x, y - p.y }; }

        constexpr Point2& operator+=(const Vec2& v) noexcept { x += v.x; y += v.y; return *this; }
        constexpr Point2& operator-=(const Vec2& v) noexcept { x -= v.x; y -= v.y; return *this; }
    };

    // AABB (Axis-Aligned Bounding Box)
    struct RectF
    {
        double x{ 0 }, y{ 0 }, w{ 0 }, h{ 0 };

        constexpr RectF() = default;
        constexpr RectF(double x_, double y_, double w_, double h_) noexcept
            : x(x_), y(y_), w(w_), h(h_)
        {
        }

        [[nodiscard]] constexpr double left()   const noexcept { return x; }
        [[nodiscard]] constexpr double right()  const noexcept { return x + w; }
        [[nodiscard]] constexpr double top()    const noexcept { return y; }
        [[nodiscard]] constexpr double bottom() const noexcept { return y + h; }
        [[nodiscard]] constexpr Point2 center() const noexcept { return { x + w * 0.5, y + h * 0.5 }; }

        [[nodiscard]] constexpr bool intersects(const RectF& o) const noexcept
        {
            return !(right() <= o.left() || o.right() <= left() ||
                bottom() <= o.top() || o.bottom() <= top());
        }
        [[nodiscard]] constexpr RectF movedBy(const Vec2& v) const noexcept { return { x + v.x, y + v.y, w, h }; }
    };

    // Tile coordinate conversion (scroll offset handling is done separately)
    [[nodiscard]] inline int ToTileIndex(double pos_px, int tile_px) noexcept
    {
        // Out of bounds = negative direction is also OK with floor (consistency in physics)
        return static_cast<int>(std::floor(pos_px / static_cast<double>(tile_px)));
    }

    // AABB overlap test (a.k.a. RectF::intersects)
    [[nodiscard]] constexpr bool overlap(const RectF& a, const RectF& b) noexcept
    {
        return !(a.right() <= b.left() || b.right() <= a.left() ||
            a.bottom() <= b.top() || b.bottom() <= a.top());
    }
}