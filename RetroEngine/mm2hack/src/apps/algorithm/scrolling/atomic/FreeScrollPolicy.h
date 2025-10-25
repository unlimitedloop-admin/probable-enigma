//==============================================================================
// 
//  Project: mm2hack
//  ***.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "apps/graphics/bg/RoomGraphAdapter.h"
#include "apps/mod/CoordinateTypes.h"
#include "Scroll.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    // 自由スクロールポリシー
    class FreeScrollPolicy final : public IScrollPolicy
    {
        using RoomGraphAdapter = graphics::bg::RoomGraphAdapter;

    public:
        FreeScrollPolicy(RoomGraphAdapter& g, int tilePx,
            int deadzoneW = 64, int deadzoneH = 64)
            : _g(g), _ts(tilePx), _dzW(deadzoneW), _dzH(deadzoneH)
        {
        }

        bool Update(const mm2hack::apps::mod::RectF& p,
            Camera& cam,
            size_t& pageIndex,
            double /*dt*/) override
        {
            using namespace mm2hack::apps::mod;

            // 1) デッドゾーン矩形
            const Scalar cx = cam.x + cam.vw * 0.5;
            const Scalar cy = cam.y + cam.vh * 0.5;
            const RectF dead(cx - _dzW * 0.5, cy - _dzH * 0.5, _dzW, _dzH);

            // 2) プレイヤーがデッドゾーンからはみ出た分だけカメラ追従
            if (p.left() < dead.left())   cam.x -= (dead.left() - p.left());
            if (p.right() > dead.right()) cam.x += (p.right() - dead.right());
            if (p.top() < dead.top())     cam.y -= (dead.top() - p.top());
            if (p.bottom() > dead.bottom()) cam.y += (p.bottom() - dead.bottom());

            // 3) ページ境界での遷移（左右）
            bool changed = false;
            const Scalar pageW = static_cast<Scalar>(_ts * 16);
            const Scalar pageH = static_cast<Scalar>(_ts * 15);

            // 左境界を越えた & 左へ遷移可能？
            while (cam.x < 0.0)
            {
                if (!_g.CanGoLeft(static_cast<int16_t>(pageIndex))) { cam.x = 0.0; break; }
                auto nb = _g.NeighborLeft(static_cast<int16_t>(pageIndex)); if (!nb) break;
                pageIndex = static_cast<size_t>(_g.PageIndexOf(*nb));
                cam.x += pageW;
                changed = true;
            }
            // 右境界
            while (cam.x + cam.vw > pageW)
            {
                if (!_g.CanGoRight(static_cast<int16_t>(pageIndex))) { cam.x = pageW - cam.vw; break; }
                auto nb = _g.NeighborRight(static_cast<int16_t>(pageIndex)); if (!nb) break;
                pageIndex = static_cast<size_t>(_g.PageIndexOf(*nb));
                cam.x -= pageW;
                changed = true;
            }
            // 上
            while (cam.y < 0.0)
            {
                if (!_g.CanGoUp(static_cast<int16_t>(pageIndex))) { cam.y = 0.0; break; }
                auto nb = _g.NeighborUp(static_cast<int16_t>(pageIndex)); if (!nb) break;
                pageIndex = static_cast<size_t>(_g.PageIndexOf(*nb));
                cam.y += pageH;
                changed = true;
            }
            // 下
            while (cam.y + cam.vh > pageH)
            {
                if (!_g.CanGoDown(static_cast<int16_t>(pageIndex))) { cam.y = pageH - cam.vh; break; }
                auto nb = _g.NeighborDown(static_cast<int16_t>(pageIndex)); if (!nb) break;
                pageIndex = static_cast<size_t>(_g.PageIndexOf(*nb));
                cam.y -= pageH;
                changed = true;
            }

            return changed;
        }

    private:
        RoomGraphAdapter& _g;
        int _ts;
        int _dzW, _dzH;
    };
}