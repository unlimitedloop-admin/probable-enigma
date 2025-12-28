#include "pch.h"

#include "ScrollController.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "core/winapi/WindowManager.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    static inline int MaxX(int view_w) { return view_w - 1; }
    static inline int MaxY(int view_h) { return view_h - 1; }

    void ScrollController::Update(const foundation::math::Vec2& input_delta)
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        // Fixed page animation: only progress
        if (_anim.Active())
        {
            const auto dir = _anim.State().dir;
            const bool finished = _anim.TickAndInterpolate(dir, page_w, page_h, _params.view_w, _params.view_h, _target_pos);
            if (finished)
            {
                _page_index = _anim.State().to_index;
                _cam.x = 0.0; _cam.y = 0.0;
                _anim.Reset();
            }
            return;
        }

        // Start fixed scroll if requested (before free-scroll updates)
        if (_pending_fixed.has_value())
        {
            const auto dir = *_pending_fixed;
            _pending_fixed.reset();

            // Resolve neighbor based on dir.
            // TODO: Refactor externally as a ScrollController method! ;) 
            auto tryStart = [&](PageScroll::Dir d) -> bool
                {
                    std::size_t from = _page_index;
                    int to_idx = -1;

                    switch (d)
                    {
                    case PageScroll::Dir::Right:
                    {
                        const auto kind = _rules.RightType(from);
                        const int16_t rr = _rules.RightRoom(from);
                        to_idx = (rr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(rr)) : -1;
                        if (!IsFixedScroll(kind)) return false;
                        break;
                    }
                    case PageScroll::Dir::Left:
                    {
                        const auto kind = _rules.LeftType(from);
                        const int16_t lr = _rules.LeftRoom(from);
                        to_idx = (lr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(lr)) : -1;
                        if (!IsFixedScroll(kind)) return false;
                        break;
                    }
                    case PageScroll::Dir::Down:
                    {
                        const auto kind = _rules.DownType(from);
                        const int16_t dr = _rules.DownRoom(from);
                        to_idx = (dr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(dr)) : -1;
                        if (!IsFixedScroll(kind)) return false;
                        break;
                    }
                    case PageScroll::Dir::Up:
                    {
                        const auto kind = _rules.UpType(from);
                        const int16_t ur = _rules.UpRoom(from);
                        to_idx = (ur >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(ur)) : -1;
                        if (!IsFixedScroll(kind)) return false;
                        break;
                    }
                    default:
                        return false;
                    }

                    _cam.x = 0.0;
                    _cam.y = 0.0;
                    _anim.Start(d, from, static_cast<std::size_t>(to_idx));
                    return true;
                };

            (void)tryStart(dir);
            // If start succeeded, next frame Update() will run TickAndInterpolate (_anim.Active() has true).
            // Even if it failed, we just fall through to free scroll.
        }

        // Free scroll (only if not animating)
        if (input_delta.x != 0.0) { updateAxisX_(input_delta.x); }
        if (input_delta.y != 0.0) { updateAxisY_(input_delta.y); }

        updateViewState_();
    }

    void ScrollController::Render()
    {
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        if (_anim.Active())
        {
            // Fixed page scroll animation in progress
            _renderer.DrawAnimation(_anim.State(), _anim.State().from_index, _anim.State().to_index);
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

    bool ScrollController::RequestFixedScroll(PageScroll::Dir dir) noexcept
    {
        if (_anim.Active())
        {
            return false;
        }

        // If already pending, keep first request (or overwrite; your choice).
        if (_pending_fixed.has_value())
        {
            return false;
        }

        _pending_fixed = dir;
        return true;
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
            // TODO: Fixed scrolling algorithm
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

    void ScrollController::updateAxisX_(double /*remain*/)
    {
        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

        const double crossX = _object_pos.x;
        const double targetWorldX = _target_pos.x;

        auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);
        double targetLocalX = targetWorldX - origin.x;
        double desired = targetLocalX - crossX;

        auto can_left_of = [&](std::size_t page) -> bool
            {
                const auto k = _rules.LeftType(page);
                const int16_t room = _rules.LeftRoom(page);
                const int idx = (room >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(room)) : -1;
                return IsAllowedFree(k) && idx >= 0;
            };

        auto can_right_of = [&](std::size_t page) -> bool
            {
                const auto k = _rules.RightType(page);
                const int16_t room = _rules.RightRoom(page);
                const int idx = (room >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(room)) : -1;
                return IsAllowedFree(k) && idx >= 0;
            };

        // --- Across-page normalization (may change _page_index) ---
        while (desired < 0.0)
        {
            const auto kind = _rules.LeftType(_page_index);
            const int16_t lr = _rules.LeftRoom(_page_index);
            const int l_idx = (lr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(lr)) : -1;

            if (!IsAllowedFree(kind) || l_idx < 0)
            {
                // No left neighbor: do not go past the left edge.
                desired = 0.0;
                break;
            }

            _page_index = static_cast<std::size_t>(l_idx);

            origin = _rules.PageOriginPx(_page_index, page_w, page_h);
            targetLocalX = targetWorldX - origin.x;
            desired = targetLocalX - crossX;
        }

        while (desired >= static_cast<double>(page_w))
        {
            const auto kind = _rules.RightType(_page_index);
            const int16_t rr = _rules.RightRoom(_page_index);
            const int r_idx = (rr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(rr)) : -1;

            if (!IsAllowedFree(kind) || r_idx < 0)
            {
                // No right neighbor: do not go past the right edge.
                desired = 0.0;
                break;
            }

            _page_index = static_cast<std::size_t>(r_idx);

            origin = _rules.PageOriginPx(_page_index, page_w, page_h);
            targetLocalX = targetWorldX - origin.x;
            desired = targetLocalX - crossX;
        }

        // Re-evaluate adjacency on the FINAL page index.
        const bool canLeft = can_left_of(_page_index);
        const bool canRight = can_right_of(_page_index);

        // --- Key rule: if there is no adjacent room in that direction, NEVER expose that side. ---
        if (!canRight && desired > 0.0)
        {
            desired = 0.0;
        }
        if (!canLeft && desired < 0.0)
        {
            desired = 0.0;
        }

        // --- Keep cam within page, but only wrap when the corresponding neighbor exists ---
        if (desired < 0.0)
        {
            desired = canLeft ? (desired + static_cast<double>(page_w)) : 0.0;
        }
        else if (desired >= static_cast<double>(page_w))
        {
            desired = canRight ? (desired - static_cast<double>(page_w)) : 0.0;
        }

        // Safety clamp
        if (desired < 0.0 || desired >= static_cast<double>(page_w))
        {
            desired = 0.0;
        }

        _cam.x = desired;
    }

    void ScrollController::updateAxisY_(double /*remain*/)
    {
        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

        const double crossY = _object_pos.y;
        const double targetWorldY = _target_pos.y;

        auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);
        double targetLocalY = targetWorldY - origin.y;
        double desired = targetLocalY - crossY;

        auto can_up_of = [&](std::size_t page) -> bool
            {
                const auto k = _rules.UpType(page);
                const int16_t room = _rules.UpRoom(page);
                const int idx = (room >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(room)) : -1;
                return IsAllowedFree(k) && idx >= 0;
            };

        auto can_down_of = [&](std::size_t page) -> bool
            {
                const auto k = _rules.DownType(page);
                const int16_t room = _rules.DownRoom(page);
                const int idx = (room >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(room)) : -1;
                return IsAllowedFree(k) && idx >= 0;
            };

        // --- Across-page normalization (may change _page_index) ---
        while (desired < 0.0)
        {
            const auto kind = _rules.UpType(_page_index);
            const int16_t ur = _rules.UpRoom(_page_index);
            const int u_idx = (ur >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(ur)) : -1;

            if (!IsAllowedFree(kind) || u_idx < 0)
            {
                // No up neighbor: do not go above current page.
                desired = 0.0;
                break;
            }

            _page_index = static_cast<std::size_t>(u_idx);

            origin = _rules.PageOriginPx(_page_index, page_w, page_h);
            targetLocalY = targetWorldY - origin.y;
            desired = targetLocalY - crossY;
        }

        while (desired >= static_cast<double>(page_h))
        {
            const auto kind = _rules.DownType(_page_index);
            const int16_t dr = _rules.DownRoom(_page_index);
            const int d_idx = (dr >= 0) ? _rules.ToPageIndex(static_cast<uint8_t>(dr)) : -1;

            if (!IsAllowedFree(kind) || d_idx < 0)
            {
                // No down neighbor: do not go below current page.
                desired = 0.0;
                break;
            }

            _page_index = static_cast<std::size_t>(d_idx);

            origin = _rules.PageOriginPx(_page_index, page_w, page_h);
            targetLocalY = targetWorldY - origin.y;
            desired = targetLocalY - crossY;
        }

        // Re-evaluate adjacency on the FINAL page index.
        const bool canUp = can_up_of(_page_index);
        const bool canDown = can_down_of(_page_index);

        // --- Key rule: if there is no adjacent room in that direction, NEVER expose that side. ---
        // This prevents "base BG color" from appearing when renderer expects neighbor room.
        if (!canDown && desired > 0.0)
        {
            desired = 0.0;
        }
        if (!canUp && desired < 0.0)
        {
            desired = 0.0;
        }

        // --- Keep cam within page, but only wrap when the corresponding neighbor exists ---
        if (desired < 0.0)
        {
            desired = canUp ? (desired + static_cast<double>(page_h)) : 0.0;
        }
        else if (desired >= static_cast<double>(page_h))
        {
            desired = canDown ? (desired - static_cast<double>(page_h)) : 0.0;
        }

        // Safety clamp
        if (desired < 0.0 || desired >= static_cast<double>(page_h))
        {
            desired = 0.0;
        }

        _cam.y = desired;
    }

    void ScrollController::drawNeighbors_()
    {
        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

        const int ox = -static_cast<int>(_cam.x);
        const int oy = -static_cast<int>(_cam.y);

        const auto is_allowed = [](ScrollKind k) { return IsAllowedFree(k); };

        // RIGHT
        if (ox + page_w < _params.view_w)
        {
            const auto k = _rules.RightType(_page_index);
            const int16_t room = _rules.RightRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox + page_w, oy);
            }
        }
        // DOWN
        if (oy + page_h < _params.view_h)
        {
            const auto k = _rules.DownType(_page_index);
            const int16_t room = _rules.DownRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox, oy + page_h);
            }
        }
        // RIGHT DOWN
        if (ox + page_w < _params.view_w && oy + page_h < _params.view_h)
        {
            const auto kr = _rules.RightType(_page_index);
            const int16_t rr = _rules.RightRoom(_page_index);
            const int r_idx = _rules.ToPageIndex(static_cast<uint8_t>(rr));
            if (IsAllowedFree(kr) && rr >= 0 && r_idx >= 0)
            {
                const auto kd = _rules.DownType(static_cast<std::size_t>(r_idx));
                const int16_t rd = _rules.DownRoom(static_cast<std::size_t>(r_idx));
                const int rd_idx = _rules.ToPageIndex(static_cast<uint8_t>(rd));
                if (IsAllowedFree(kd) && rd >= 0 && rd_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(rd_idx), ox + page_w, oy + page_h);
                }
            }
        }
        // LEFT
        if (ox > 0)
        {
            const auto k = _rules.LeftType(_page_index);
            const int16_t room = _rules.LeftRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox - page_w, oy);
            }
        }
        // UP
        if (oy > 0)
        {
            const auto k = _rules.UpType(_page_index);
            const int16_t room = _rules.UpRoom(_page_index);
            const int idx = _rules.ToPageIndex(static_cast<uint8_t>(room));
            if (is_allowed(k) && room >= 0 && idx >= 0)
            {
                _renderer.DrawPage(static_cast<std::size_t>(idx), ox, oy - page_h);
            }
        }
        // LEFT UP
        if (ox > 0 && oy > 0)
        {
            const auto kl = _rules.LeftType(_page_index);
            const int16_t lr = _rules.LeftRoom(_page_index);
            const int l_idx = _rules.ToPageIndex(static_cast<uint8_t>(lr));
            if (IsAllowedFree(kl) && lr >= 0 && l_idx >= 0)
            {
                const auto ku = _rules.UpType(static_cast<std::size_t>(l_idx));
                const int16_t lu = _rules.UpRoom(static_cast<std::size_t>(l_idx));
                const int lu_idx = _rules.ToPageIndex(static_cast<uint8_t>(lu));
                if (IsAllowedFree(ku) && lu >= 0 && lu_idx >= 0)
                {
                    _renderer.DrawPage(static_cast<std::size_t>(lu_idx), ox - page_w, oy - page_h);
                }
            }
        }
    }

    void ScrollController::updateViewState_()
    {
        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

        const auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);

        // Convert world cam to ViewState cam
        _viewState.camX = origin.x + _cam.x;
        _viewState.camY = origin.y + _cam.y;
        _viewState.viewW = _params.view_w;
        _viewState.viewH = _params.view_h;
    }
}