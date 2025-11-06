#include "pch.h"

#include "PageScrollAnimator.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    bool PageScrollAnimator::TickAndInterpolate(PageDir dir, int page_w, int page_h, int view_w, int view_h, Vec2& object_pos) noexcept
    {
        if (!_pg.active) { return false; }

        const int max_x = view_w - 1;
        const int max_y = view_h - 1;

        _pg.progress += _pg.speed;
        const double tx = std::clamp(_pg.progress / static_cast<double>(page_w), 0.0, 1.0);
        const double ty = std::clamp(_pg.progress / static_cast<double>(page_h), 0.0, 1.0);

        switch (dir)
        {
        case PageDir::Right: object_pos.x = (1.0 - tx) * max_x; break;  // 255 -> 0
        case PageDir::Left:  object_pos.x = tx * max_x; break;          // 0 -> 255
        case PageDir::Down:  object_pos.y = (1.0 - ty) * max_y; break;  // 239 -> 0
        case PageDir::Up:    object_pos.y = ty * max_y; break;          // 0 -> 239
        default: break;
        }

        const double need = (dir == PageDir::Left || dir == PageDir::Right) ? page_w : page_h;
        if (_pg.progress >= need)
        {
            // Final adjustment.
            switch (dir)
            {
            case PageDir::Right: object_pos.x = 0; break;
            case PageDir::Left:  object_pos.x = max_x; break;
            case PageDir::Down:  object_pos.y = 0; break;
            case PageDir::Up:    object_pos.y = max_y; break;
            default: break;
            }
            _pg.active = false; _pg.progress = 0.0;
            return true;
        }
        return false;
    }
}