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
#include <cstdlib>

namespace mm2hack::apps::mod
{
    using Scalar = double;

    // 数学ユーティリティ
    constexpr Scalar kEps = static_cast<Scalar>(1e-9);

    [[nodiscard]] inline bool NearlyZero(Scalar v, Scalar eps = kEps) noexcept
    {
        return std::abs(v) <= eps;
    }

    [[nodiscard]] inline bool NearlyEqual(Scalar a, Scalar b, Scalar eps = kEps) noexcept
    {
        return std::abs(a - b) <= eps;
    }

    // 2D ベクトル（速度/差分/方向）
    struct Vec2
    {
        Scalar x{ 0 }, y{ 0 };

        constexpr Vec2() = default;
        constexpr Vec2(Scalar x_, Scalar y_) noexcept : x(x_), y(y_) {}

        [[nodiscard]] constexpr Vec2 operator+(const Vec2& o) const noexcept { return { x + o.x, y + o.y }; }
        [[nodiscard]] constexpr Vec2 operator-(const Vec2& o) const noexcept { return { x - o.x, y - o.y }; }
        [[nodiscard]] constexpr Vec2 operator*(Scalar s) const noexcept { return { x * s, y * s }; }
        [[nodiscard]] constexpr Vec2 operator/(Scalar s) const noexcept { return { x / s, y / s }; }

        constexpr Vec2& operator+=(const Vec2& o) noexcept { x += o.x; y += o.y; return *this; }
        constexpr Vec2& operator-=(const Vec2& o) noexcept { x -= o.x; y -= o.y; return *this; }
        constexpr Vec2& operator*=(Scalar s) noexcept { x *= s; y *= s; return *this; }
        constexpr Vec2& operator/=(Scalar s) noexcept { x /= s; y /= s; return *this; }

        [[nodiscard]] Scalar length() const noexcept { return std::sqrt(x * x + y * y); }
        [[nodiscard]] Scalar lengthSq() const noexcept { return x * x + y * y; }
        [[nodiscard]] Vec2 normalized(Scalar eps = kEps) const noexcept
        {
            const auto len = length();
            return (len <= eps) ? Vec2{ 0,0 } : Vec2{ x / len, y / len };
        }
        static constexpr Vec2 Zero() noexcept { return {}; }
    };
    inline constexpr Vec2 operator*(Scalar s, const Vec2& v) noexcept { return { v.x * s, v.y * s }; }

    // 2D 点（位置）。点＋ベクトル＝点 / 点−点＝ベクトル
    struct Point2
    {
        Scalar x{ 0 }, y{ 0 };

        constexpr Point2() = default;
        constexpr Point2(Scalar x_, Scalar y_) noexcept : x(x_), y(y_) {}

        [[nodiscard]] constexpr Point2 operator+(const Vec2& v) const noexcept { return { x + v.x, y + v.y }; }
        [[nodiscard]] constexpr Point2 operator-(const Vec2& v) const noexcept { return { x - v.x, y - v.y }; }
        [[nodiscard]] constexpr Vec2   operator-(const Point2& p) const noexcept { return { x - p.x, y - p.y }; }

        constexpr Point2& operator+=(const Vec2& v) noexcept { x += v.x; y += v.y; return *this; }
        constexpr Point2& operator-=(const Vec2& v) noexcept { x -= v.x; y -= v.y; return *this; }
    };

    // AABB（当たり箱）
    struct RectF
    {
        Scalar x{ 0 }, y{ 0 }, w{ 0 }, h{ 0 };

        constexpr RectF() = default;
        constexpr RectF(Scalar x_, Scalar y_, Scalar w_, Scalar h_) noexcept
            : x(x_), y(y_), w(w_), h(h_)
        {
        }

        [[nodiscard]] constexpr Scalar left()   const noexcept { return x; }
        [[nodiscard]] constexpr Scalar right()  const noexcept { return x + w; }
        [[nodiscard]] constexpr Scalar top()    const noexcept { return y; }
        [[nodiscard]] constexpr Scalar bottom() const noexcept { return y + h; }

        [[nodiscard]] constexpr Point2 center() const noexcept { return { x + w * 0.5, y + h * 0.5 }; }

        [[nodiscard]] constexpr bool intersects(const RectF& o) const noexcept
        {
            return !(right() <= o.left() || o.right() <= left() ||
                bottom() <= o.top() || o.bottom() <= top());
        }
        [[nodiscard]] constexpr RectF movedBy(const Vec2& v) const noexcept { return { x + v.x, y + v.y, w, h }; }
    };

    // タイル座標変換（スクロールオフセット対応は別で足し引きする）
    [[nodiscard]] inline int ToTileIndex(Scalar pos_px, int tile_px) noexcept
    {
        // 画面外＝負方向も floor で OK（物理の一貫性）
        return static_cast<int>(std::floor(pos_px / static_cast<Scalar>(tile_px)));
    }

    // 差分タイプ（入力・外力など用途を明示）
    struct Diff2
    {
        Vec2 delta{};

        [[nodiscard]] bool any(Scalar eps = kEps) const noexcept
        {
            return !NearlyZero(delta.x, eps) || !NearlyZero(delta.y, eps);
        }

        void reset() noexcept { delta = Vec2::Zero(); }
    };
}