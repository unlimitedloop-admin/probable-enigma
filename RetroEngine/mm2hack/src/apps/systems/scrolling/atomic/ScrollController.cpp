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
            fx.fixedActive = true;

            const auto dir = _anim.State().dir;
            const double prevProg = _anim.State().progress;

            const bool finished = _anim.Tick(dir, page_w, page_h); // progress += speed(=4)

            const double need = (dir == ScrlDir::Left || dir == ScrlDir::Right)
                ? static_cast<double>(page_w)
                : static_cast<double>(page_h);

            // Clamp current progress to [0..need] for stable dProg
            const double currProg = finished ? need : std::min(_anim.State().progress, need);
            const double dProg = std::max(0.0, currProg - prevProg);

            // carry is proportional to camera progress
            const double carry = (need > 0.0) ? (dProg * (_carryTotalPx / need)) : 0.0;

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
                // freeze-draw snap as you already do
                PageScroll snap = _anim.State();
                snap.progress = need;
                snap.active = true;

                _freeze_draw = snap;
                _freezeFrames = kFreezeOnEnd;

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
            const FixedScrollRequest req = *_pending_fixed; // copy
            _pending_fixed.reset();

            if (tryStartFixed_(req, page_w, page_h))
            {
                _freezeFrames = kFreezeOnStart;
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

    bool ScrollController::RequestFixedScroll(const FixedScrollRequest& req) noexcept
    {
        if (_anim.Active()) return false;
        if (_pending_fixed.has_value()) return false;

        _pending_fixed = req;
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

    bool ScrollController::IsFreezeFrames() const noexcept
    {
        return _freezeFrames > 0;
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

        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

        const auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);
        _view_world = origin;  // Start at top-left of the page.
        _cam = { 0.0, 0.0 };
    }

    void ScrollController::updateAxisX_(double remain)
    {
        const double crossX = _object_pos.x;
        const double worldX = _target_pos.x;

        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

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

        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

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
        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

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
        const int page_w = _params.tile_px * config::SystemConfig::kTileCountX;
        const int page_h = _params.tile_px * config::SystemConfig::kTileCountY;

        for (;;)
        {
            const auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);

            // If viewWorld is outside current page, move page_index to the neighbor page.
            if (_view_world.x < origin.x)
            {
                const int next = resolveNextIndexX_(_rules, _page_index, -1);
                if (next < 0)
                {
                    _view_world.x = origin.x;
                    break;
                }
                _page_index = static_cast<std::size_t>(next);
                continue;
            }

            if (_view_world.x >= origin.x + static_cast<double>(page_w))
            {
                const int next = resolveNextIndexX_(_rules, _page_index, +1);
                if (next < 0)
                {
                    _view_world.x = origin.x;
                    break;
                }
                _page_index = static_cast<std::size_t>(next);
                continue;
            }

            if (_view_world.y < origin.y)
            {
                const int next = resolveNextIndexY_(_rules, _page_index, -1);
                if (next < 0)
                {
                    _view_world.y = origin.y;
                    break;
                }
                _page_index = static_cast<std::size_t>(next);
                continue;
            }

            if (_view_world.y >= origin.y + static_cast<double>(page_h))
            {
                const int next = resolveNextIndexY_(_rules, _page_index, +1);
                if (next < 0)
                {
                    _view_world.y = origin.y;
                    break;
                }
                _page_index = static_cast<std::size_t>(next);
                continue;
            }

            // Now viewWorld is inside current page.
            break;
        }

        // Update cam (local offset) from viewWorld.
        const auto origin = _rules.PageOriginPx(_page_index, page_w, page_h);
        _cam.x = _view_world.x - origin.x;
        _cam.y = _view_world.y - origin.y;

        // Safety clamp: cam must stay within [0..page)
        if (_cam.x < 0.0) _cam.x = 0.0;
        if (_cam.y < 0.0) _cam.y = 0.0;
        if (_cam.x >= static_cast<double>(page_w)) _cam.x = 0.0;
        if (_cam.y >= static_cast<double>(page_h)) _cam.y = 0.0;

        if (_cam.y < 0.0 || _cam.y >= static_cast<double>(page_h))
        {
            // log error: cam out of range
            THROW_EXCEPTION(L"ScrollController: cam.y out of bounds after normalization", kClassName);
        }
    }

    void ScrollController::updateViewState_()
    {
        _viewState.camX = _cam.x;
        _viewState.camY = _cam.y;

        _viewState.viewWorldX = _view_world.x;
        _viewState.viewWorldY = _view_world.y;

        _viewState.pageIndex = _page_index;
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

    bool ScrollController::tryStartFixed_(const FixedScrollRequest& req, int page_w, int page_h)
    {
        if (req.dir == PageScroll::Dir::None) return false;

        const std::size_t from = _page_index;
        const auto to = resolveFixedNeighbor_(req.dir, from);
        if (!to) return false;

        _carryTotalPx = 2.0 * std::max(0.0, req.edgeGapPx); // ★あなたの仕様
        _cam.x = 0.0;
        _cam.y = 0.0;

        _anim.Start(req.dir, from, *to);
        return true;
    }
}