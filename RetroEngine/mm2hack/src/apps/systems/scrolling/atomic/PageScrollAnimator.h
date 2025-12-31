//==============================================================================
// 
//  Project: mm2hack
//  PageScrollAnimator.h
// 
//  A simple animator for page scrolling.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // Animator for page scrolling effects
    class PageScrollAnimator
    {
        using PageDir = PageScroll::Dir;
        using Vec2 = foundation::math::Vec2;

    public:
        PageScrollAnimator() = default;
        ~PageScrollAnimator() = default;

        void Reset() noexcept { _pg = {}; }
        bool Active() const noexcept { return _pg.active; }
        const PageScroll& State() const noexcept { return _pg; }

        // Progress update and player position interpolation (0..pageW/H)
        // Returns: true if completed
        bool Tick(PageDir dir, int page_w, int page_h) noexcept;

        // Start page scroll animation
        void Start(PageDir dir, std::size_t from_idx, std::size_t to_idx) noexcept
        {
            _pg.active = true;
            _pg.dir = dir; _pg.from_index = from_idx; _pg.to_index = to_idx; _pg.progress = 0.0;
        }

        // Set scroll speed (px/frame)
        void SetSpeed(double px_per_frame) noexcept { _pg.speed = px_per_frame; }

    private:
        const std::wstring kClassName{ L"PageScrollAnimator" };

        PageScroll _pg{};
    };
}