#include "pch.h"

#include "TileCollision.h"

#include <cmath>
#include <cstdint>
#include "apps/graphics/bg/BGTileManager.h"

namespace
{
    inline int floordiv(int a, int b)
    {
        return static_cast<int>(std::floor(static_cast<float>(a) / static_cast<float>(b)));
    }
}

namespace mm2hack::apps::algorithm::physics
{
    using mm2hack::apps::algorithm::physics::Attr;
    using mm2hack::apps::algorithm::physics::AABB;
    using mm2hack::apps::algorithm::physics::ResolveResult;
    using mm2hack::apps::algorithm::physics::TileCollisionResolver;

    uint8_t TileCollisionResolver::getAttrByPx(int px, int py, float ofs_x_px, float ofs_y_px) const noexcept
    {
        // 画面スクロール時の BG オフセット（今は 0 のままでもOK）
        const int gx = px - static_cast<int>(ofs_x_px);
        const int gy = py - static_cast<int>(ofs_y_px);

        const int tx = floordiv(gx, _ts);
        const int ty = floordiv(gy, _ts);
        if (tx < 0 || ty < 0 || tx >= _mw || ty >= _mh) return static_cast<uint8_t>(Attr::Solid); // 画面外は壁扱い
        return _mgr.GetTileAttribute(tx, ty);
    }

    bool TileCollisionResolver::isSolid(int tx, int ty) const noexcept
    {
        // xy→属性問い合わせは BGTileManager の API 都合で px 経由に統一したいが、
        // ここは内部ユースのみなので簡易に 1px 中心で px 化
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx(px, py, 0, 0) == static_cast<uint8_t>(Attr::Solid);
    }
    bool TileCollisionResolver::isOneWay(int tx, int ty) const noexcept
    {
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx(px, py, 0, 0) == static_cast<uint8_t>(Attr::OneWay);
    }
    bool TileCollisionResolver::isLadder(int tx, int ty) const noexcept
    {
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx(px, py, 0, 0) == static_cast<uint8_t>(Attr::Ladder);
    }
    bool TileCollisionResolver::isSpike(int tx, int ty) const noexcept
    {
        const int px = tx * _ts + _ts / 2;
        const int py = ty * _ts + _ts / 2;
        return getAttrByPx(px, py, 0, 0) == static_cast<uint8_t>(Attr::Spike);
    }

    ResolveResult TileCollisionResolver::Resolve(AABB& b, float& vx, float& vy,
        const bool allow_one_way_pass_up,
        float ofs_x, float ofs_y) const
    {
        ResolveResult rr{};
        // --- X 軸 ---
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
                if (isSpike(tx, ty))  rr.touched_spike = true;
                if (isLadder(tx, ty)) rr.on_ladder = true;

                if (isSolid(tx, ty))
                {
                    if (dir > 0) b.x = static_cast<float>(tx * _ts) - b.w;
                    else         b.x = static_cast<float>((tx + 1) * _ts);
                    vx = 0.0f;
                    break;
                }
            }
        }

        // --- Y 軸 ---
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
                if (isSpike(tx, ty))  rr.touched_spike = true;
                if (isLadder(tx, ty)) rr.on_ladder = true;

                // 下向き移動のときだけ OneWay に衝突させる
                const bool hit_oneway = (dir > 0.0f) && isOneWay(tx, ty);
                const bool hit_solid = isSolid(tx, ty);

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

        // はしごタイルに重なっているだけでもフラグは立てる（入力で昇降させる）
        if (!rr.on_ladder)
        {
            const int cx = static_cast<int>(std::floor((b.x + b.w * 0.5f) / _ts));
            const int cy = static_cast<int>(std::floor((b.y + b.h * 0.5f) / _ts));
            rr.on_ladder = isLadder(cx, cy);
        }

        return rr;
    }
}