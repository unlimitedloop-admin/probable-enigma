#include "pch.h"

#include "ScrollController.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/ViewState.h"
#include "core/winapi/WindowManager.h"  // Use for draw reference lines for debug HUD
#include "FreeScrollDriver.h"
#include "IScrollRuleProvider.h"
#include "MapRenderer2D.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    using foundation::math::Vec2;

    static inline int MaxX(int view_w) { return view_w - 1; }
    static inline int MaxY(int view_h) { return view_h - 1; }

    ScrollController::ScrollController(IScrollRuleProvider& rules, MapRenderer2D& renderer, Params params)
        : _rules(rules)
        , _renderer(renderer)
        , _params(params)
        , _neighbor(_rules)
        , _free(_rules, _neighbor, FreeScrollDriver::Params{ _params.tile_px, _tileX, _tileY })
        , _fixed_driver(_anim, _neighbor)
    {
    }

    ScrollEffect ScrollController::Update(const Vec2& input_delta)
    {
        ScrollEffect fx{};

        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        // 1) Fixed scroll has priority (freeze/anim/pending)
        if (_fixed_driver.Update(page_w, page_h, _cam, _page_index, _fixed_freeze, fx))
        {
            updateViewState_();
            return fx;
        }

        // 2) Free scroll
        _free.Update(input_delta, _object_pos, _target_pos, _page_index, _view_world, _cam);

        // 3) View state update
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

    void ScrollController::SyncWithObjectCenter(
        const Vec2& object_center, bool has_adj_x, bool has_adj_y,
        const Vec2& screen_px, const Vec2& map_px, apps::systems::view::ViewState& out_view)
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
}