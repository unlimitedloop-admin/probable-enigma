#include "pch.h"

#include "ScrollController.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "core/winapi/WindowManager.h"  // Use for draw reference lines for debug HUD
#include "IScrollRuleProvider.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    static inline int MaxX(int view_w) { return view_w - 1; }
    static inline int MaxY(int view_h) { return view_h - 1; }

    ScrollEffect ScrollController::Update(const foundation::math::Vec2& input_delta)
    {
        ScrollEffect fx{};

        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        if (_fixed_driver.Update(page_w, page_h, _cam, _page_index, _fixed_freeze, fx))
        {
            updateViewState_();
            return fx;
        }

        updateFreeScroll_(input_delta);

        updateViewState_();
        return fx;
    }

    void ScrollController::Render()
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        if (_anim.Active())
        {
            _renderer.DrawAnimation(_anim.State(), _anim.State().from_index, _anim.State().to_index);
        }
        else if (_fixed_freeze.DrawSnapshot().has_value())
        {
            const auto& snap = _fixed_freeze.DrawSnapshot();
            _renderer.DrawAnimation(*snap, snap->from_index, snap->to_index);
        }
        else
        {
            // Free scroll: current page + only necessary adjacent pages
            const int ox = -static_cast<int>(_cam.x);
            const int oy = -static_cast<int>(_cam.y);
            _renderer.DrawPage(_page_index, ox, oy);
            drawNeighbors_();
        }
    }

    bool ScrollController::RequestFixedScroll(const FixedScrollRequest& req) noexcept
    {
        return _fixed_driver.Request(req);
    }

    bool ScrollController::IsFixedScrollLocked() const noexcept
    {
        return _fixed_driver.IsLocked() || _fixed_freeze.IsActive();
    }

    bool ScrollController::IsScrollLocked() const noexcept
    {
        return IsFixedScrollLocked();
    }

    FixedScrollMeasure ScrollController::CurrentPageBoundsWorld() const noexcept
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        const auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);

        FixedScrollMeasure measure{};
        measure.fromBounds.leftX = origin.x;
        measure.fromBounds.rightX = origin.x + static_cast<double>(page_w);
        measure.fromBounds.topY = origin.y;
        measure.fromBounds.bottomY = origin.y + static_cast<double>(page_h);
        measure.pageOriginPx = origin;

        return measure;
    }

    bool ScrollController::IsFreezeFrames() const noexcept
    {
        return _fixed_freeze.IsActive();
    }

    std::optional<std::size_t> ScrollController::ResolveFixedNeighbor(PageScroll::Dir dir, std::size_t from) const
    {
        return resolveFixedNeighbor_(dir, from);
    }

    void ScrollController::SyncWithObjectCenter(const Vec2& object_center, bool has_adj_x, bool has_adj_y, const Vec2& screen_px, const Vec2& map_px, ViewState& out_view)
    {
        _object_pos = object_center; // Object position sync

        auto clamp = [](double v, double lo, double hi) { return std::max(lo, std::min(v, hi)); };

        if (IsAllowedFree(_mode))
        {
            _cam.x = has_adj_x ? (screen_px.x * 0.5)
                : clamp(object_center.x, screen_px.x * 0.5, map_px.x - screen_px.x * 0.5);
            _cam.y = has_adj_y ? (screen_px.y * 0.5)
                : clamp(object_center.y, screen_px.y * 0.5, map_px.y - screen_px.y * 0.5);
        }
        else
        {
            // Fixed page: keep page pinned. Animation is handled by DrawAnimation.
            if (!_anim.Active())
            {
                _cam.x = 0.0;
                _cam.y = 0.0;
            }
        }

        out_view.camX = _cam.x;
        out_view.camY = _cam.y;
    }

    void ScrollController::DebugHudRender(bool show) const
    {
        if (!show) return;

        auto& wm = core::winapi::WindowManager::GetInstance();
        auto viewerRate = wm.GetViewerRate();

        // Debug crosshair
        ::DxLib::DrawLine(static_cast<int>(_object_pos.x * viewerRate), 0, static_cast<int>(_object_pos.x * viewerRate), static_cast<int>(_params.view_h * viewerRate), 0xFFFF0000, 2);
        ::DxLib::DrawLine(0, static_cast<int>(_object_pos.y * viewerRate), static_cast<int>(_params.view_w * viewerRate), static_cast<int>(_object_pos.y * viewerRate), 0xFFFF0000, 2);
    }

    void ScrollController::SetPageIndex(std::size_t idx) noexcept
    {
        _page_index = idx;

        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        const auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);
        _view_world = origin;  // Start at top-left of the page.
        _cam = { 0.0, 0.0 };
    }

    void ScrollController::updateAxisX_(double remain)
    {
        const double crossX = _object_pos.x;
        const double worldX = _target_pos.x;

        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);

        const bool hasLeft = (resolveNextIndexX_(_rules, _page_index, -1) >= 0);
        const bool hasRight = (resolveNextIndexX_(_rules, _page_index, +1) >= 0);

        const double screenX = worldX - _view_world.x;

        if (remain < 0.0)
        {
            // Moving left
            if (screenX <= crossX)
            {
                const double next = _view_world.x + remain; // negative

                if (hasLeft)
                {
                    _view_world.x = next;
                }
                else
                {
                    // No LEFT neighbor: do not go left of origin, but allow returning.
                    _view_world.x = std::max(next, origin.x);
                }
            }
        }
        else if (remain > 0.0)
        {
            // Moving right
            if (screenX >= crossX)
            {
                const double next = _view_world.x + remain; // positive

                if (hasRight)
                {
                    _view_world.x = next;
                }
                else
                {
                    // No RIGHT neighbor: do not go right of origin, but allow returning.
                    _view_world.x = std::min(next, origin.x);
                }
            }
        }

        normalizeViewWorldToPage_();
    }

    void ScrollController::updateAxisY_(double remain)
    {
        const double crossY = _object_pos.y;
        const double worldY = _target_pos.y;

        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        // "Current" origin of the page_index (before normalization).
        auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);

        // Is there an adjacent page in each direction (free-scroll allowed)?
        const bool hasUp = (resolveNextIndexY_(_rules, _page_index, -1) >= 0);
        const bool hasDown = (resolveNextIndexY_(_rules, _page_index, +1) >= 0);

        // Current on-screen Y of the target (world - viewWorld).
        const double screenY = worldY - _view_world.y;

        if (remain < 0.0)
        {
            // Moving up: follow only when above (or at) the cross line.
            if (screenY <= crossY)
            {
                const double next = _view_world.y + remain; // remain is negative

                if (hasUp)
                {
                    // Allowed to expose UP neighbor.
                    _view_world.y = next;
                }
                else
                {
                    // No UP neighbor:
                    // Allow only "returning toward origin" (i.e., do not go above origin).
                    _view_world.y = std::max(next, origin.y);
                }
            }
        }
        else if (remain > 0.0)
        {
            // Moving down: follow only when below (or at) the cross line.
            if (screenY >= crossY)
            {
                const double next = _view_world.y + remain; // remain is positive

                if (hasDown)
                {
                    // Allowed to expose DOWN neighbor.
                    _view_world.y = next;
                }
                else
                {
                    // No DOWN neighbor:
                    // Allow only "returning toward origin" (i.e., do not go below origin).
                    _view_world.y = std::min(next, origin.y);
                }
            }
        }

        // After changing viewWorld, normalize page/cam consistently.
        normalizeViewWorldToPage_();
    }

    int ScrollController::resolveNextIndexX_(const IScrollRuleProvider& rules, const std::size_t page_index, const int dir)
    {
        if (dir == 0) return -1;

        const auto kind = (dir > 0) ? rules.RightType(page_index) : rules.LeftType(page_index);
        const int16_t room = (dir > 0) ? rules.RightRoom(page_index) : rules.LeftRoom(page_index);
        if (!IsAllowedFree(kind) || room < 0) return -1;

        const int idx = rules.ToPageIndex(static_cast<uint8_t>(room));
        return (idx >= 0) ? idx : -1;
    }

    int ScrollController::resolveNextIndexY_(const IScrollRuleProvider& rules, const std::size_t page_index, const int dir)
    {
        if (dir == 0) return -1;

        const auto kind = (dir > 0) ? rules.DownType(page_index) : rules.UpType(page_index);
        const int16_t room = (dir > 0) ? rules.DownRoom(page_index) : rules.UpRoom(page_index);
        if (!IsAllowedFree(kind) || room < 0) return -1;

        const int idx = rules.ToPageIndex(static_cast<uint8_t>(room));
        return (idx >= 0) ? idx : -1;
    }

    void ScrollController::drawNeighbors_()
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        const int ox = -static_cast<int>(_cam.x);
        const int oy = -static_cast<int>(_cam.y);

        const auto is_allowed = [](ScrollKind k) noexcept { return IsAllowedFree(k); };

        auto to_page = [&](int16_t room) -> int
            {
                if (room < 0) return -1;
                return _rules.ToPageIndex(static_cast<uint8_t>(room));
            };

        const bool needLeft = (ox > 0);
        const bool needRight = (ox + page_w < _params.view_w);
        const bool needUp = (oy > 0);
        const bool needDown = (oy + page_h < _params.view_h);

        // RIGHT
        if (needRight)
        {
            const auto k = _rules.RightType(_page_index);
            const int idx = to_page(_rules.RightRoom(_page_index));
            if (is_allowed(k) && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox + page_w, oy);
            }
        }

        // LEFT
        if (needLeft)
        {
            const auto k = _rules.LeftType(_page_index);
            const int idx = to_page(_rules.LeftRoom(_page_index));
            if (is_allowed(k) && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox - page_w, oy);
            }
        }

        // DOWN
        if (needDown)
        {
            const auto k = _rules.DownType(_page_index);
            const int idx = to_page(_rules.DownRoom(_page_index));
            if (is_allowed(k) && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox, oy + page_h);
            }
        }

        // UP
        if (needUp)
        {
            const auto k = _rules.UpType(_page_index);
            const int idx = to_page(_rules.UpRoom(_page_index));
            if (is_allowed(k) && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox, oy - page_h);
            }
        }

        // RIGHT+DOWN
        if (needRight && needDown)
        {
            const auto kr = _rules.RightType(_page_index);
            const int r_idx = to_page(_rules.RightRoom(_page_index));
            if (is_allowed(kr) && r_idx >= 0)
            {
                const auto kd = _rules.DownType(static_cast<std::size_t>(r_idx));
                const int rd_idx = to_page(_rules.DownRoom(static_cast<std::size_t>(r_idx)));
                if (is_allowed(kd) && rd_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(rd_idx), ox + page_w, oy + page_h);
                }
            }
        }

        // RIGHT+UP
        if (needRight && needUp)
        {
            const auto kr = _rules.RightType(_page_index);
            const int r_idx = to_page(_rules.RightRoom(_page_index));
            if (is_allowed(kr) && r_idx >= 0)
            {
                const auto ku = _rules.UpType(static_cast<std::size_t>(r_idx));
                const int ru_idx = to_page(_rules.UpRoom(static_cast<std::size_t>(r_idx)));
                if (is_allowed(ku) && ru_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(ru_idx), ox + page_w, oy - page_h);
                }
            }
        }

        // LEFT+DOWN
        if (needLeft && needDown)
        {
            const auto kl = _rules.LeftType(_page_index);
            const int l_idx = to_page(_rules.LeftRoom(_page_index));
            if (is_allowed(kl) && l_idx >= 0)
            {
                const auto kd = _rules.DownType(static_cast<std::size_t>(l_idx));
                const int ld_idx = to_page(_rules.DownRoom(static_cast<std::size_t>(l_idx)));
                if (is_allowed(kd) && ld_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(ld_idx), ox - page_w, oy + page_h);
                }
            }
        }

        // LEFT+UP
        if (needLeft && needUp)
        {
            const auto kl = _rules.LeftType(_page_index);
            const int l_idx = to_page(_rules.LeftRoom(_page_index));
            if (is_allowed(kl) && l_idx >= 0)
            {
                const auto ku = _rules.UpType(static_cast<std::size_t>(l_idx));
                const int lu_idx = to_page(_rules.UpRoom(static_cast<std::size_t>(l_idx)));
                if (is_allowed(ku) && lu_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(lu_idx), ox - page_w, oy - page_h);
                }
            }
        }
    }

    void ScrollController::normalizeViewWorldToPage_()
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        // Compute cam relative to current base page.
        auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);
        double camX = _view_world.x - origin.x;
        double camY = _view_world.y - origin.y;

        // Helper: whether neighbor is drawable by FREE scroll rules
        auto has_left = [&]() { return resolveNextIndexX_(_rules, _page_index, -1) >= 0; };
        auto has_right = [&]() { return resolveNextIndexX_(_rules, _page_index, +1) >= 0; };
        auto has_up = [&]() { return resolveNextIndexY_(_rules, _page_index, -1) >= 0; };
        auto has_down = [&]() { return resolveNextIndexY_(_rules, _page_index, +1) >= 0; };

        // 1) If cam goes beyond one page, shift page_index accordingly.
        //    (Allow negative cam and positive cam; we normalize by moving base page.)
        for (;;)
        {
            bool moved = false;

            // Left overflow: camX < -page_w => base page should move LEFT
            while (camX < -static_cast<double>(page_w))
            {
                const int next = resolveNextIndexX_(_rules, _page_index, -1);
                if (next < 0) { camX = -static_cast<double>(page_w); break; }
                _page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(_page_index, page_w, page_h);
                camX += static_cast<double>(page_w);
                moved = true;
            }

            // Right overflow: camX >= +page_w => base page should move RIGHT
            while (camX >= static_cast<double>(page_w))
            {
                const int next = resolveNextIndexX_(_rules, _page_index, +1);
                if (next < 0) { camX = static_cast<double>(page_w) - 1.0; break; }
                _page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(_page_index, page_w, page_h);
                camX -= static_cast<double>(page_w);
                moved = true;
            }

            // Up overflow
            while (camY < -static_cast<double>(page_h))
            {
                const int next = resolveNextIndexY_(_rules, _page_index, -1);
                if (next < 0) { camY = -static_cast<double>(page_h); break; }
                _page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(_page_index, page_w, page_h);
                camY += static_cast<double>(page_h);
                moved = true;
            }

            // Down overflow
            while (camY >= static_cast<double>(page_h))
            {
                const int next = resolveNextIndexY_(_rules, _page_index, +1);
                if (next < 0) { camY = static_cast<double>(page_h) - 1.0; break; }
                _page_index = static_cast<std::size_t>(next);
                origin = _rules.PageOriginPx(_page_index, page_w, page_h);
                camY -= static_cast<double>(page_h);
                moved = true;
            }

            if (!moved) break;
        }

        // 2) IMPORTANT FIX:
        //    Do not expose a side if the corresponding FREE neighbor doesn't exist.
        //    In your renderer convention:
        //      camX > 0 means "right neighbor area appears" (ox = -camX).
        //      camX < 0 means "left neighbor area appears".
        if (!has_right() && camX > 0.0) camX = 0.0;
        if (!has_left() && camX < 0.0) camX = 0.0;

        if (!has_down() && camY > 0.0) camY = 0.0;
        if (!has_up() && camY < 0.0) camY = 0.0;

        // 3) Commit cam and view_world consistently.
        _cam.x = camX;
        _cam.y = camY;

        _view_world.x = origin.x + _cam.x;
        _view_world.y = origin.y + _cam.y;
    }

    void ScrollController::updateViewState_()
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        // Base: current page origin + free cam
        auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);
        double viewWorldX = origin.x + _cam.x;
        double viewWorldY = origin.y + _cam.y;

        // --- Fixed scroll / freeze-draw: camera is driven by animation progress ---
        const PageScroll* pg = nullptr;
        if (_anim.Active()) pg = &_anim.State();
        else if (auto& snap = _fixed_freeze.DrawSnapshot(); snap.has_value()) pg = &(*snap);

        if (pg && pg->active)
        {
            // IMPORTANT: during animation we draw FROM page (from_index)
            auto fromOrigin = _rules.PageOriginPx(pg->from_index, page_w, page_h);
            viewWorldX = fromOrigin.x;
            viewWorldY = fromOrigin.y;

            const double prog = pg->progress;

            switch (pg->dir)
            {
            case PageScroll::Dir::Right: viewWorldX += prog; break;
            case PageScroll::Dir::Left:  viewWorldX -= prog; break;
            case PageScroll::Dir::Down:  viewWorldY += prog; break;
            case PageScroll::Dir::Up:    viewWorldY -= prog; break;
            default: break;
            }

            // optional: keep cam=0 while anim, that's fine
            // _cam.x = 0; _cam.y = 0;
        }

        _viewState.viewWorldX = viewWorldX;
        _viewState.viewWorldY = viewWorldY;
        _viewState.camX = _cam.x;
        _viewState.camY = _cam.y;
    }

    std::optional<std::size_t> ScrollController::resolveFixedNeighbor_(PageScroll::Dir dir, std::size_t from) const
    {
        auto roomToIndex = [&](int16_t room) -> std::optional<std::size_t>
            {
                if (room < 0) return std::nullopt;
                const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
                if (idx < 0) return std::nullopt;
                return static_cast<std::size_t>(idx);
            };

        switch (dir)
        {
        case PageScroll::Dir::Right:
        {
            if (!IsFixedScroll(_rules.RightType(from))) return std::nullopt;
            return roomToIndex(_rules.RightRoom(from));
        }
        case PageScroll::Dir::Left:
        {
            if (!IsFixedScroll(_rules.LeftType(from))) return std::nullopt;
            return roomToIndex(_rules.LeftRoom(from));
        }
        case PageScroll::Dir::Down:
        {
            if (!IsFixedScroll(_rules.DownType(from))) return std::nullopt;
            return roomToIndex(_rules.DownRoom(from));
        }
        case PageScroll::Dir::Up:
        {
            if (!IsFixedScroll(_rules.UpType(from))) return std::nullopt;
            return roomToIndex(_rules.UpRoom(from));
        }
        default:
            return std::nullopt;
        }
    }

    void ScrollController::updateFreeScroll_(const foundation::math::Vec2& input_delta) noexcept
    {
        if (input_delta.x != 0.0)
        {
            updateAxisX_(input_delta.x);
        }

        if (input_delta.y != 0.0)
        {
            updateAxisY_(input_delta.y);
        }
    }
}