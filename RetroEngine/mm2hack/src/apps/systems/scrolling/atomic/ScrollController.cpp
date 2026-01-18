#include "pch.h"

#include "ScrollController.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/ViewState.h"
#include "FreeScrollDriver.h"
#include "IScrollRuleProvider.h"
#include "MapRenderer2D.h"
#include "ScrollTypes.h"

#include "core/winapi/WindowManager.h"  // Use for draw reference lines for debug HUD

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
        const PageScroll* pg = activeFixedScrollState_();
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
        }

        _viewState.viewWorldX = viewWorldX;
        _viewState.viewWorldY = viewWorldY;
        _viewState.camX = _cam.x;
        _viewState.camY = _cam.y;
    }

    const PageScroll* ScrollController::activeFixedScrollState_() const noexcept
    {
        if (_anim.Active()) return &_anim.State();

        const auto& snap = _fixed_freeze.DrawSnapshot();
        if (snap.has_value()) return &(*snap);

        return nullptr;
    }

    void ScrollController::drawNeighbors_()
    {
        using Dir = PageScroll::Dir;

        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;

        const int ox = -static_cast<int>(_cam.x);
        const int oy = -static_cast<int>(_cam.y);

        // Check if neighboring page areas are exposed on the screen (necessary determination)
        const bool needLeft = (ox > 0);
        const bool needRight = (ox + page_w < _params.view_w);
        const bool needUp = (oy > 0);
        const bool needDown = (oy + page_h < _params.view_h);

        // Create need mask
        enum NeedMask : unsigned
        {
            kNeedNone = 0,
            kNeedL = 1u << 0,
            kNeedR = 1u << 1,
            kNeedU = 1u << 2,
            kNeedD = 1u << 3,
        };

        unsigned needMask = kNeedNone;
        if (needLeft)  needMask |= kNeedL;
        if (needRight) needMask |= kNeedR;
        if (needUp)    needMask |= kNeedU;
        if (needDown)  needMask |= kNeedD;

        // Resolve one step (adjacent page): return std::nullopt if not possible
        auto step = [&](std::size_t from, Dir d) -> std::optional<std::size_t>
            {
                int idx = -1;

                switch (d)
                {
                case Dir::Right: idx = _neighbor.ResolveNextIndexX(from, +1); break;
                case Dir::Left:  idx = _neighbor.ResolveNextIndexX(from, -1); break;
                case Dir::Down:  idx = _neighbor.ResolveNextIndexY(from, +1); break;
                case Dir::Up:    idx = _neighbor.ResolveNextIndexY(from, -1); break;
                default: break;
                }

                if (idx < 0)
                {
                    return std::nullopt;
                }

                return static_cast<std::size_t>(idx);
            };

        struct DrawReq
        {
            unsigned need_mask{};
            int dx_pages{};
            int dy_pages{};
            Dir path1{ Dir::None };
            Dir path2{ Dir::None }; // 2nd step for corner. Straight lines remain None.
        };

        // Draw requests (4 straight + 4 diagonal)
        // dx_pages/dy_pages indicate how many pages to offset from the current page.
        constexpr std::array<DrawReq, 8> kReqs =
        {
            DrawReq{ kNeedR,          +1,  0, Dir::Right, Dir::None },
            DrawReq{ kNeedL,          -1,  0, Dir::Left,  Dir::None },
            DrawReq{ kNeedD,           0, +1, Dir::Down,  Dir::None },
            DrawReq{ kNeedU,           0, -1, Dir::Up,    Dir::None },

            DrawReq{ kNeedR | kNeedD, +1, +1, Dir::Right, Dir::Down },
            DrawReq{ kNeedR | kNeedU, +1, -1, Dir::Right, Dir::Up   },
            DrawReq{ kNeedL | kNeedD, -1, +1, Dir::Left,  Dir::Down },
            DrawReq{ kNeedL | kNeedU, -1, -1, Dir::Left,  Dir::Up   },
        };

        for (const auto& req : kReqs)
        {
            // If not needed, skip.
            if ((needMask & req.need_mask) != req.need_mask)
            {
                continue;
            }

            // Resolve adjacent pages along the path (1 step for straight lines, 2 steps for diagonals).
            std::optional<std::size_t> idx = step(_page_index, req.path1);
            if (!idx.has_value())
            {
                continue;
            }

            if (req.path2 != Dir::None)
            {
                idx = step(*idx, req.path2);
                if (!idx.has_value())
                {
                    continue;
                }
            }

            // Render the resolved neighboring page at the correct offset.
            _renderer.DrawPage(*idx, ox + req.dx_pages * page_w, oy + req.dy_pages * page_h);
        }
    }
}