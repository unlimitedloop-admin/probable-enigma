#include "pch.h"

#include "FreeScrollPolicy.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/resources/bg/RoomGraphAdapter.h"
#include "ScrollController.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    bool FreeScrollPolicy::Update(const RectF& p, Camera& cam, size_t& pageIndex, double /*dt*/)
    {
        // Calculate camera position based on player position `p`.
        const Scalar cx = cam.x + cam.vw * 0.5;
        const Scalar cy = cam.y + cam.vh * 0.5;
        const RectF dead(cx - _dzW * 0.5, cy - _dzH * 0.5, _dzW, _dzH);

        // Relative to deadzone.
        if (p.left() < dead.left())     cam.x -= (dead.left() - p.left());
        if (p.right() > dead.right())   cam.x += (p.right() - dead.right());
        if (p.top() < dead.top())       cam.y -= (dead.top() - p.top());
        if (p.bottom() > dead.bottom()) cam.y += (p.bottom() - dead.bottom());

        // Out of bounds check & page transition.
        bool changed = false;
        const Scalar pageW = static_cast<Scalar>(_ts * 16);
        const Scalar pageH = static_cast<Scalar>(_ts * 15);

        // Over the left boundary
        while (cam.x < 0.0)
        {
            if (!_g.CanGoLeft(static_cast<int16_t>(pageIndex))) { cam.x = 0.0; break; }
            auto nb = _g.NeighborLeft(static_cast<int16_t>(pageIndex)); if (!nb) break;
            pageIndex = static_cast<size_t>(_g.PageIndexOf(*nb));
            cam.x += pageW;
            changed = true;
        }
        // Over the right boundary
        while (cam.x + cam.vw > pageW)
        {
            if (!_g.CanGoRight(static_cast<int16_t>(pageIndex))) { cam.x = pageW - cam.vw; break; }
            auto nb = _g.NeighborRight(static_cast<int16_t>(pageIndex)); if (!nb) break;
            pageIndex = static_cast<size_t>(_g.PageIndexOf(*nb));
            cam.x -= pageW;
            changed = true;
        }
        // Over the top
        while (cam.y < 0.0)
        {
            if (!_g.CanGoUp(static_cast<int16_t>(pageIndex))) { cam.y = 0.0; break; }
            auto nb = _g.NeighborUp(static_cast<int16_t>(pageIndex)); if (!nb) break;
            pageIndex = static_cast<size_t>(_g.PageIndexOf(*nb));
            cam.y += pageH;
            changed = true;
        }
        // Over the bottom
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
}