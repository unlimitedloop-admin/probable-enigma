#include "pch.h"

#include "ScrollController.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
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

    namespace
    {
        constexpr std::uint32_t kMaximumPageIndex = 65535;
        constexpr double kMaximumCoordinate = 1000000.0;

        bool IsFiniteCoordinate(double value) noexcept
        {
            return std::isfinite(value) && std::abs(value) <= kMaximumCoordinate;
        }

        bool IsValidDirection(PageScroll::Dir dir) noexcept
        {
            return dir >= PageScroll::Dir::None && dir <= PageScroll::Dir::Up;
        }

        bool IsValidPageScroll(const PageScroll& state) noexcept
        {
            return IsValidDirection(state.dir) &&
                std::isfinite(state.progress) && state.progress >= 0.0 &&
                state.progress <= kMaximumCoordinate &&
                std::isfinite(state.speed) && state.speed > 0.0 &&
                state.speed <= kMaximumCoordinate &&
                state.from_index <= kMaximumPageIndex &&
                state.to_index <= kMaximumPageIndex &&
                ((state.active && state.dir != PageScroll::Dir::None &&
                  state.from_index != state.to_index) ||
                 (!state.active && state.dir == PageScroll::Dir::None &&
                  state.progress == 0.0));
        }

        bool WritePageScroll(core::save::StateWriter& writer, const PageScroll& state)
        {
            return writer.WriteBool(state.active) &&
                writer.WriteU8(static_cast<std::uint8_t>(state.dir)) &&
                writer.WriteF64(state.progress) && writer.WriteF64(state.speed) &&
                writer.WriteU32(static_cast<std::uint32_t>(state.from_index)) &&
                writer.WriteU32(static_cast<std::uint32_t>(state.to_index));
        }

        bool ReadPageScroll(core::save::StateReader& reader, PageScroll& state)
        {
            bool active{};
            std::uint8_t direction{};
            double progress{};
            double speed{};
            std::uint32_t from_index{};
            std::uint32_t to_index{};
            if (!reader.ReadBool(active) || !reader.ReadU8(direction) ||
                !reader.ReadF64(progress) || !reader.ReadF64(speed) ||
                !reader.ReadU32(from_index) || !reader.ReadU32(to_index))
            {
                return false;
            }
            state = PageScroll{
                active,
                static_cast<PageScroll::Dir>(direction),
                progress,
                speed,
                static_cast<std::size_t>(from_index),
                static_cast<std::size_t>(to_index)
            };
            return IsValidPageScroll(state);
        }
    }

    bool ScrollControllerState::Save(core::save::StateWriter& writer) const
    {
        if (!IsValid()) return false;
        if (!writer.WriteU8(static_cast<std::uint8_t>(mode)) ||
            !writer.WriteU32(page_index) ||
            !writer.WriteF64(view_world.x) || !writer.WriteF64(view_world.y) ||
            !writer.WriteF64(object_pos.x) || !writer.WriteF64(object_pos.y) ||
            !writer.WriteF64(target_pos.x) || !writer.WriteF64(target_pos.y) ||
            !writer.WriteF64(camera_x) || !writer.WriteF64(camera_y) ||
            !WritePageScroll(writer, animator) || !writer.WriteBool(pending.has_value()))
        {
            return false;
        }
        if (pending.has_value() &&
            (!writer.WriteBool(pending->available) ||
             !writer.WriteU8(static_cast<std::uint8_t>(pending->dir)) ||
             !writer.WriteF64(pending->carryTotalPx)))
        {
            return false;
        }
        if (!writer.WriteF64(carry_total_px) || !writer.WriteI32(freeze_frames) ||
            !writer.WriteBool(freeze_draw.has_value()))
        {
            return false;
        }
        return !freeze_draw.has_value() || WritePageScroll(writer, *freeze_draw);
    }

    bool ScrollControllerState::Load(core::save::StateReader& reader)
    {
        ScrollControllerState loaded{};
        std::uint8_t mode_value{};
        bool has_pending{};
        if (!reader.ReadU8(mode_value) || !reader.ReadU32(loaded.page_index) ||
            !reader.ReadF64(loaded.view_world.x) || !reader.ReadF64(loaded.view_world.y) ||
            !reader.ReadF64(loaded.object_pos.x) || !reader.ReadF64(loaded.object_pos.y) ||
            !reader.ReadF64(loaded.target_pos.x) || !reader.ReadF64(loaded.target_pos.y) ||
            !reader.ReadF64(loaded.camera_x) || !reader.ReadF64(loaded.camera_y) ||
            !ReadPageScroll(reader, loaded.animator) || !reader.ReadBool(has_pending))
        {
            return false;
        }
        loaded.mode = static_cast<ScrollMode>(mode_value);
        if (has_pending)
        {
            bool available{};
            std::uint8_t direction{};
            double carry{};
            if (!reader.ReadBool(available) || !reader.ReadU8(direction) ||
                !reader.ReadF64(carry))
            {
                return false;
            }
            loaded.pending = FixedScrollRequest{
                available, static_cast<PageScroll::Dir>(direction), carry
            };
        }
        bool has_freeze_draw{};
        if (!reader.ReadF64(loaded.carry_total_px) ||
            !reader.ReadI32(loaded.freeze_frames) ||
            !reader.ReadBool(has_freeze_draw))
        {
            return false;
        }
        if (has_freeze_draw)
        {
            PageScroll draw{};
            if (!ReadPageScroll(reader, draw)) return false;
            loaded.freeze_draw = draw;
        }
        if (!loaded.IsValid()) return false;
        *this = loaded;
        return true;
    }

    bool ScrollControllerState::IsValid() const noexcept
    {
        if (mode < ScrollMode::None || mode > ScrollMode::TargetFollow ||
            page_index > kMaximumPageIndex ||
            !IsFiniteCoordinate(view_world.x) || !IsFiniteCoordinate(view_world.y) ||
            !IsFiniteCoordinate(object_pos.x) || !IsFiniteCoordinate(object_pos.y) ||
            !IsFiniteCoordinate(target_pos.x) || !IsFiniteCoordinate(target_pos.y) ||
            !IsFiniteCoordinate(camera_x) || !IsFiniteCoordinate(camera_y) ||
            !IsValidPageScroll(animator) ||
            !std::isfinite(carry_total_px) || carry_total_px < 0.0 ||
            carry_total_px > kMaximumCoordinate ||
            (!animator.active && carry_total_px != 0.0) ||
            freeze_frames < 0 || freeze_frames > ScrollFreezeState::kFreezeOnStart)
        {
            return false;
        }
        if (pending.has_value() &&
            (!pending->available || !IsValidDirection(pending->dir) ||
             pending->dir == PageScroll::Dir::None ||
             !std::isfinite(pending->carryTotalPx) || pending->carryTotalPx < 0.0 ||
             pending->carryTotalPx > kMaximumCoordinate || animator.active ||
             freeze_frames != 0))
        {
            return false;
        }
        if (freeze_draw.has_value() &&
            (freeze_frames == 0 || !freeze_draw->active || !IsValidPageScroll(*freeze_draw)))
        {
            return false;
        }
        if (freeze_frames != 0 && !freeze_draw.has_value() && !animator.active)
        {
            return false;
        }
        return true;
    }

    ScrollControllerState ScrollController::CaptureState() const noexcept
    {
        return ScrollControllerState{
            _mode,
            static_cast<std::uint32_t>(
                _page_index <= kMaximumPageIndex ? _page_index : kMaximumPageIndex + 1U),
            _view_world,
            _object_pos,
            _target_pos,
            _cam.x,
            _cam.y,
            _anim.State(),
            _fixed_driver.Pending(),
            _fixed_driver.CarryTotalPx(),
            _fixed_freeze.Frames(),
            _fixed_freeze.DrawSnapshot()
        };
    }

    bool ScrollController::RestoreState(const ScrollControllerState& state) noexcept
    {
        if (!state.IsValid()) return false;
        const int page_w = _params.tile_px * _tileX;
        const int page_h = _params.tile_px * _tileY;
        const auto is_runtime_valid_scroll = [&](const PageScroll& scroll)
            {
                if (!scroll.active) return true;
                const double maximum_progress = IsHorizontal(scroll.dir)
                    ? static_cast<double>(page_w)
                    : static_cast<double>(page_h);
                const auto expected = _neighbor.ResolveFixedNeighbor(scroll.dir, scroll.from_index);
                return scroll.progress <= maximum_progress && expected.has_value() &&
                    *expected == scroll.to_index;
            };
        if (!is_runtime_valid_scroll(state.animator) ||
            (state.freeze_draw.has_value() &&
             !is_runtime_valid_scroll(*state.freeze_draw)))
        {
            return false;
        }
        _mode = state.mode;
        _page_index = state.page_index;
        _view_world = state.view_world;
        _object_pos = state.object_pos;
        _target_pos = state.target_pos;
        _cam.x = state.camera_x;
        _cam.y = state.camera_y;
        _anim.RestoreState(state.animator);
        _fixed_driver.RestoreState(state.pending, state.carry_total_px);
        _fixed_freeze.RestoreState(state.freeze_frames, state.freeze_draw);
        updateViewState_();
        return true;
    }

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
        _viewState.pageIndex = static_cast<int>(_page_index);
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
