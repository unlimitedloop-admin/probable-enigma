#include "pch.h"

#include "PageScrollAnimator.h"

#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    bool PageScrollAnimator::Tick(PageDir dir, int page_w, int page_h) noexcept
    {
        if (!_pg.active) return false;

        _pg.progress += _pg.speed;

        const double need =
            (dir == PageDir::Left || dir == PageDir::Right) ? static_cast<double>(page_w)
            : static_cast<double>(page_h);

        if (_pg.progress >= need)
        {
            _pg.active = false;
            _pg.progress = 0.0;
            return true;
        }
        return false;
    }
}