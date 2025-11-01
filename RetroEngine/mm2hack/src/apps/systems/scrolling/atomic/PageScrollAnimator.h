//==============================================================================
// 
//  Project: mm2hack
//  PageScrollAnimator.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // 固定ページスクロールの演出を担当
    class PageScrollAnimator
    {
    public:
        void Reset() noexcept { _pg = {}; }
        bool Active() const noexcept { return _pg.active; }
        const PageScroll& State() const noexcept { return _pg; }

        // 進捗更新とプレイヤー座標の補間（0..pageW/H）
        // 戻り値: 完了したら true
        bool TickAndInterpolate(PageScroll::Dir dir, int page_w, int page_h,
            int view_w, int view_h, foundation::math::Vec2& object_pos) noexcept;

        // 開始
        void Start(PageScroll::Dir dir, std::size_t from_idx, std::size_t to_idx) noexcept
        {
            _pg.active = true;
            _pg.dir = dir; _pg.from_index = from_idx; _pg.to_index = to_idx; _pg.progress = 0.0;
        }

        void SetSpeed(double px_per_frame) noexcept { _pg.speed = px_per_frame; }

    private:
        PageScroll _pg{};
    };
}