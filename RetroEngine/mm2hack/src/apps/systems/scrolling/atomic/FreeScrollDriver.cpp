#include "pch.h"

#include "FreeScrollDriver.h"

#include "Camera.h"
#include "IScrollRuleProvider.h"
#include "ScrollNeighborResolver.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    void FreeScrollDriver::Update(
        const Vec2& input_delta, const Vec2& object_pos, const Vec2& target_pos,
        std::size_t& page_index, Vec2& view_world, Camera& cam) const
    {
        if (input_delta.x != 0.0)
        {
            updateAxisX_(input_delta.x, object_pos, target_pos, page_index, view_world, cam);
        }

        if (input_delta.y != 0.0)
        {
            updateAxisY_(input_delta.y, object_pos, target_pos, page_index, view_world, cam);
        }
    }

    void FreeScrollDriver::updateAxisX_(
        const double remain, const Vec2& object_pos, const Vec2& target_pos,
        std::size_t& page_index, Vec2& view_world, Camera& cam) const
    {
        const double crossX = object_pos.x;
        const double worldX = target_pos.x;

        const int page_w = _params.tile_px * _params.tileX;
        const int page_h = _params.tile_px * _params.tileY;
        auto origin = _rules.PageOriginPx(page_index, page_w, page_h);

        const bool hasLeft = (_resolver.ResolveNextIndexX(page_index, -1) >= 0);
        const bool hasRight = (_resolver.ResolveNextIndexX(page_index, +1) >= 0);

        const double screenX = worldX - view_world.x;

        if (remain < 0.0)
        {
            if (screenX <= crossX)
            {
                const double next = view_world.x + remain;

                if (hasLeft)
                {
                    view_world.x = next;
                }
                else
                {
                    view_world.x = std::max(next, origin.x);
                }
            }
        }
        else if (remain > 0.0)
        {
            if (screenX >= crossX)
            {
                const double next = view_world.x + remain;

                if (hasRight)
                {
                    view_world.x = next;
                }
                else
                {
                    view_world.x = std::min(next, origin.x);
                }
            }
        }

        normalizeViewWorldToPage_(page_index, view_world, cam);
    }

    void FreeScrollDriver::updateAxisY_(
        const double remain, const Vec2& object_pos, const Vec2& target_pos,
        std::size_t& page_index, Vec2& view_world, Camera& cam) const
    {
        const double crossY = object_pos.y;
        const double worldY = target_pos.y;

        const int page_w = _params.tile_px * _params.tileX;
        const int page_h = _params.tile_px * _params.tileY;
        auto origin = _rules.PageOriginPx(page_index, page_w, page_h);

        const bool hasUp = (_resolver.ResolveNextIndexY(page_index, -1) >= 0);
        const bool hasDown = (_resolver.ResolveNextIndexY(page_index, +1) >= 0);

        const double screenY = worldY - view_world.y;

        if (remain < 0.0)
        {
            if (screenY <= crossY)
            {
                const double next = view_world.y + remain;

                if (hasUp)
                {
                    view_world.y = next;
                }
                else
                {
                    view_world.y = std::max(next, origin.y);
                }
            }
        }
        else if (remain > 0.0)
        {
            if (screenY >= crossY)
            {
                const double next = view_world.y + remain;

                if (hasDown)
                {
                    view_world.y = next;
                }
                else
                {
                    view_world.y = std::min(next, origin.y);
                }
            }
        }

        normalizeViewWorldToPage_(page_index, view_world, cam);
    }

    void FreeScrollDriver::normalizeViewWorldToPage_(std::size_t& page_index, Vec2& view_world, Camera& cam) const
    {
        const int page_w = _params.tile_px * _params.tileX;
        const int page_h = _params.tile_px * _params.tileY;

        auto origin = _rules.PageOriginPx(page_index, page_w, page_h);
        double camX = view_world.x - origin.x;
        double camY = view_world.y - origin.y;

        for (;;)
        {
            bool moved = false;

            while (camX < -static_cast<double>(page_w))
            {
                const int next = _resolver.ResolveNextIndexX(page_index, -1);
                if (next < 0)
                {
                    camX = -static_cast<double>(page_w);
                    break;
                }

                page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(page_index, page_w, page_h);
                camX += static_cast<double>(page_w);
                moved = true;
            }

            while (camX >= static_cast<double>(page_w))
            {
                const int next = _resolver.ResolveNextIndexX(page_index, +1);
                if (next < 0)
                {
                    camX = static_cast<double>(page_w) - 1.0;
                    break;
                }

                page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(page_index, page_w, page_h);
                camX -= static_cast<double>(page_w);
                moved = true;
            }

            while (camY < -static_cast<double>(page_h))
            {
                const int next = _resolver.ResolveNextIndexY(page_index, -1);
                if (next < 0)
                {
                    camY = -static_cast<double>(page_h);
                    break;
                }

                page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(page_index, page_w, page_h);
                camY += static_cast<double>(page_h);
                moved = true;
            }

            while (camY >= static_cast<double>(page_h))
            {
                const int next = _resolver.ResolveNextIndexY(page_index, +1);
                if (next < 0)
                {
                    camY = static_cast<double>(page_h) - 1.0;
                    break;
                }

                page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(page_index, page_w, page_h);
                camY -= static_cast<double>(page_h);
                moved = true;
            }

            if (!moved)
            {
                break;
            }
        }

        // Important fix: prevent exposing sides without neighbors.
        const bool has_left = (_resolver.ResolveNextIndexX(page_index, -1) >= 0);
        const bool has_right = (_resolver.ResolveNextIndexX(page_index, +1) >= 0);
        const bool has_up = (_resolver.ResolveNextIndexY(page_index, -1) >= 0);
        const bool has_down = (_resolver.ResolveNextIndexY(page_index, +1) >= 0);

        if (!has_right && camX > 0.0) camX = 0.0;
        if (!has_left && camX < 0.0) camX = 0.0;

        if (!has_down && camY > 0.0) camY = 0.0;
        if (!has_up && camY < 0.0) camY = 0.0;

        cam.x = camX;
        cam.y = camY;

        view_world.x = origin.x + cam.x;
        view_world.y = origin.y + cam.y;
    }
}