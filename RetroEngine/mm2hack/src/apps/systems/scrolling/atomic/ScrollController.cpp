#include "pch.h"

#include "ScrollController.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "core/winapi/WindowManager.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    static inline int MaxX(int view_w) { return view_w - 1; }
    static inline int MaxY(int view_h) { return view_h - 1; }

    ScrollEffect ScrollController::Update(const foundation::math::Vec2& input_delta)
    {
        using ScrlDir = PageScroll::Dir;
        ScrollEffect fx{};

        if (_freezeFrames > 0)
        {
            --_freezeFrames;
            if (_freezeFrames == 0) { _freeze_draw.reset(); }

            updateViewState_();
            return fx;
        }

        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        constexpr double kCamStep    = 0x04.00p0;      // 4.0 px/frame
        constexpr double kPlayerStep = 0x00.C0p0;   // 0.75 px/frame
        constexpr double kCarryRatio = kPlayerStep / kCamStep; // 0.1875

        // TODO: We'll split this into method later.
        if (_anim.Active())
        {
            ScrollEffect fx{};
            fx.fixedActive = true;

            const auto dir = _anim.State().dir;
            const double prevProg = _anim.State().progress;

            const bool finished = _anim.Tick(dir, page_w, page_h); // progress += speed(=4)

            const double currProg = finished ? static_cast<double>((dir == ScrlDir::Left || dir == ScrlDir::Right) ? page_w : page_h)
                : _anim.State().progress;

            const double dProg = currProg - prevProg;  // typically 4.0

            const double carry = dProg * kCarryRatio;  // typically 0.75

            switch (dir)
            {
            case ScrlDir::Right: fx.playerDelta.x = +carry; break;
            case ScrlDir::Left:  fx.playerDelta.x = -carry; break;
            case ScrlDir::Down:  fx.playerDelta.y = +carry; break;
            case ScrlDir::Up:    fx.playerDelta.y = -carry; break;
            default: break;
            }

            if (finished)
            {
                // Keep final frame for drawing during end-freeze
                PageScroll snap = _anim.State();          // from/to/dir/speed etc
                snap.progress = (snap.dir == PageScroll::Dir::Left || snap.dir == PageScroll::Dir::Right)
                    ? static_cast<double>(page_w) : static_cast<double>(page_h);
                snap.active = true;

                _freeze_draw = snap;                // Set freeze-draw state
                _freezeFrames = kFreezeOnEnd;       // Set freeze frames on scroll end

                // Commit new page, reset anim runtime
                _page_index = _anim.State().to_index;
                _cam.x = 0.0; _cam.y = 0.0;
                _anim.Reset();

                updateViewState_();
                return fx;
            }
            updateViewState_();
            return fx;
        }

        // Start fixed scroll if requested (before free-scroll updates)
        if (_pending_fixed.has_value())
        {
            const auto dir = *_pending_fixed;
            _pending_fixed.reset();

            auto tryStart = [&](PageScroll::Dir d) -> bool
                {
                    const std::size_t from = _page_index;

                    auto resolveToIndex = [&](int16_t room) -> int
                        {
                            if (room < 0) return -1;
                            return _rules.ToPageIndex(static_cast<uint8_t>(room));
                        };

                    int to_idx = -1;

                    switch (d)
                    {
                    case PageScroll::Dir::Right:
                    {
                        const auto kind = _rules.RightType(from);
                        if (!IsFixedScroll(kind)) return false;
                        to_idx = resolveToIndex(_rules.RightRoom(from));
                        break;
                    }
                    case PageScroll::Dir::Left:
                    {
                        const auto kind = _rules.LeftType(from);
                        if (!IsFixedScroll(kind)) return false;
                        to_idx = resolveToIndex(_rules.LeftRoom(from));
                        break;
                    }
                    case PageScroll::Dir::Down:
                    {
                        const auto kind = _rules.DownType(from);
                        if (!IsFixedScroll(kind)) return false;
                        to_idx = resolveToIndex(_rules.DownRoom(from));
                        break;
                    }
                    case PageScroll::Dir::Up:
                    {
                        const auto kind = _rules.UpType(from);
                        if (!IsFixedScroll(kind)) return false;
                        to_idx = resolveToIndex(_rules.UpRoom(from));
                        break;
                    }
                    default:
                        return false;
                    }

                    // Invalid target
                    if (to_idx < 0) return false;

                    _cam.x = 0.0;
                    _cam.y = 0.0;
                    _anim.Start(d, from, static_cast<std::size_t>(to_idx));

                    return true;
                };

            if (tryStart(dir))
            {
                _freezeFrames = kFreezeOnStart;     // Freeze frames on scroll start.
                updateViewState_();
                return fx;
            }
        }

        // Free scroll
        if (input_delta.x != 0.0) { updateAxisX_(input_delta.x); }
        if (input_delta.y != 0.0) { updateAxisY_(input_delta.y); }

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
        else if (_freeze_draw.has_value())
        {
            _renderer.DrawAnimation(*_freeze_draw, _freeze_draw->from_index, _freeze_draw->to_index);
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

    bool ScrollController::IsFixedScrollLocked() const noexcept
    {
        return _anim.Active() || _pending_fixed.has_value();
    }

    bool ScrollController::IsScrollLocked() const noexcept
    {
        return _anim.Active() || _pending_fixed.has_value() || (_freezeFrames > 0);
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
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        const auto curOrigin = _rules.PageOriginPx(_page_index, page_w, page_h);

        if (_anim.Active())
        {
            const auto& st = _anim.State();

            // from page origin (start page)
            const auto fromOrigin = _rules.PageOriginPx(st.from_index, page_w, page_h);
            const double prog = st.progress;

            double vx = fromOrigin.x;
            double vy = fromOrigin.y;

            switch (st.dir)
            {
            case PageScroll::Dir::Right: vx += prog; break;
            case PageScroll::Dir::Left:  vx -= prog; break;
            case PageScroll::Dir::Down:  vy += prog; break;
            case PageScroll::Dir::Up:    vy -= prog; break;
            default: break;
            }

            _viewState.viewWorldX = vx;
            _viewState.viewWorldY = vy;

            // Here the camera remains local to the page being animated.
            _viewState.camX = _cam.x;
            _viewState.camY = _cam.y;
            return;
        }

        // Free: page origin + local cam offset
        _viewState.viewWorldX = curOrigin.x + _cam.x;
        _viewState.viewWorldY = curOrigin.y + _cam.y;

        _viewState.camX = _cam.x;
        _viewState.camY = _cam.y;
    }
}