#include "pch.h"

#include "TileCollision.h"

#include <cmath>
#include "apps/rendering/bg/BGTileManager.h"

namespace
{
    inline int floordiv(int a, int b)
    {
        return static_cast<int>(std::floor(static_cast<float>(a) / static_cast<float>(b)));
    }
}

namespace mm2hack::apps::systems::physics
{
    using physics::Attr;
    using physics::AABB;
    using physics::ResolveResult;
    using physics::TileCollisionResolver;

    uint8_t TileCollisionResolver::getAttrByPx_(int px, int py, float ofs_x_px, float ofs_y_px) const noexcept
    {
        // BG access is tile-based, so convert px -> tx/ty
        const int gx = px - static_cast<int>(ofs_x_px);
        const int gy = py - static_cast<int>(ofs_y_px);

        const int tx = floordiv(gx, _ts);
        const int ty = floordiv(gy, _ts);
        if (tx < 0 || ty < 0 || tx >= _mw || ty >= _mh) return static_cast<uint8_t>(Attr::Solid); // The wall outside the map
        return _mgr.GetTileAttribute(tx, ty);
    }

    bool TileCollisionResolver::isSolid_(int tx, int ty) const noexcept
    {
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx_(px, py, 0, 0) == static_cast<uint8_t>(Attr::Solid);
    }
    bool TileCollisionResolver::isOneWay_(int tx, int ty) const noexcept
    {
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx_(px, py, 0, 0) == static_cast<uint8_t>(Attr::OneWay);
    }
    bool TileCollisionResolver::isLadder_(int tx, int ty) const noexcept
    {
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx_(px, py, 0, 0) == static_cast<uint8_t>(Attr::Ladder);
    }
    bool TileCollisionResolver::isSpike_(int tx, int ty) const noexcept
    {
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx_(px, py, 0, 0) == static_cast<uint8_t>(Attr::Spike);
    }

    ResolveResult TileCollisionResolver::Resolve(AABB& b, float& vx, float& vy,
        const bool allow_one_way_pass_up,
        float ofs_x, float ofs_y) const
    {
        ResolveResult rr{};
        // --- X axis ---
        if (vx != 0.0f)
        {
            b.x += vx;
            const float dir = (vx > 0.0f) ? 1.f : -1.f;

            const int left = static_cast<int>(std::floor(b.x / _ts));
            const int right = static_cast<int>(std::floor((b.x + b.w - 0.001f) / _ts));
            const int top = static_cast<int>(std::floor(b.y / _ts));
            const int bottom = static_cast<int>(std::floor((b.y + b.h - 0.001f) / _ts));

            const int tx = (dir > 0) ? right : left;
            for (int ty = top; ty <= bottom; ++ty)
            {
                if (isSpike_(tx, ty))  rr.touched_spike = true;
                if (isLadder_(tx, ty)) rr.on_ladder = true;

                if (isSolid_(tx, ty))
                {
                    if (dir > 0) b.x = static_cast<float>(tx * _ts) - b.w;
                    else         b.x = static_cast<float>((tx + 1) * _ts);
                    vx = 0.0f;
                    break;
                }
            }
        }

        // --- Y axis ---
        rr.on_ground = false;

        if (vy != 0.0f)
        {
            b.y += vy;
            const float dir = (vy > 0.0f) ? 1.f : -1.f;

            const int left = static_cast<int>(std::floor(b.x / _ts));
            const int right = static_cast<int>(std::floor((b.x + b.w - 0.001f) / _ts));
            const int top = static_cast<int>(std::floor(b.y / _ts));
            const int bottom = static_cast<int>(std::floor((b.y + b.h - 0.001f) / _ts));

            const int ty = (dir > 0) ? bottom : top;
            for (int tx = left; tx <= right; ++tx)
            {
                if (isSpike_(tx, ty))  rr.touched_spike = true;
                if (isLadder_(tx, ty)) rr.on_ladder = true;

                // Only collide with OneWay when moving down.
                const bool hit_oneway = (dir > 0.0f) && isOneWay_(tx, ty);
                const bool hit_solid = isSolid_(tx, ty);

                if (hit_solid || hit_oneway)
                {
                    if (dir > 0.0f)
                    {
                        b.y = static_cast<float>(ty * _ts) - b.h;
                        rr.on_ground = true;
                    }
                    else
                    {
                        b.y = static_cast<float>((ty + 1) * _ts);
                    }
                    vy = 0.0f;
                    break;
                }
            }
        }

        // The flag is set just by overlapping with the ladder tile (to allow climbing with input).
        if (!rr.on_ladder)
        {
            const int cx = static_cast<int>(std::floor((b.x + b.w * 0.5f) / _ts));
            const int cy = static_cast<int>(std::floor((b.y + b.h * 0.5f) / _ts));
            rr.on_ladder = isLadder_(cx, cy);
        }

        return rr;
    }
}