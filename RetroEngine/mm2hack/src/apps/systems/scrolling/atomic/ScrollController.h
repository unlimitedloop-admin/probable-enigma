//==============================================================================
// 
//  Project: mm2hack
//  ScrollController.h
// 
//  Manages scrolling behavior in 2D map systems.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/ViewState.h"
#include "Camera.h"
#include "config/SystemConfig.h"
#include "FixedScrollDriver.h"
#include "FreeScrollDriver.h"
#include "IScrollRuleProvider.h"
#include "MapRenderer2D.h"
#include "PageScrollAnimator.h"
#include "ScrollFreezeState.h"
#include "ScrollNeighborResolver.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    using foundation::math::kEps;

    // 2D difference representation
    struct Diff2
    {
        using Vec2 = foundation::math::Vec2;
        Vec2 delta{};

        [[nodiscard]] bool any(double eps = kEps) const noexcept
        {
            return !Camera::NearlyZero(delta.x, eps) || !Camera::NearlyZero(delta.y, eps);
        }

        void reset() noexcept { delta = Vec2::Zero(); }
    };

    // The main video screen policy and management of scrolling in 2D maps
    class ScrollController final
    {
        using conf      = config::SystemConfig;

    public:
        struct Params
        {
            int tile_px{ conf::kTileSize };     // Tile px
            int view_w{ conf::kScreenWidth };   // Width of the screen
            int view_h{ conf::kScreenHeight };  // Height of the screen
        };

        ScrollController(IScrollRuleProvider& rules, MapRenderer2D& renderer, Params params);

        // Free scroll / fixed animation comprehensive update
        ScrollEffect Update(const foundation::math::Vec2& input_delta);

        // Rendering
        void Render();

        // Request fixed page scroll. Returns false if rejected (no neighbor / not allowed / already animating)
        bool RequestFixedScroll(const FixedScrollRequest& req) noexcept;
        // Check if fixed scroll is locked (animating or pending)
        [[nodiscard]] bool IsFixedScrollLocked() const noexcept;
        // Check if any scroll is locked (fixed animating/pending or freeze)
        [[nodiscard]] bool IsScrollLocked() const noexcept;
        // Get current page bounds in world coordinates
        [[nodiscard]] FixedScrollMeasure CurrentPageBoundsWorld() const noexcept;
        // Check if in freeze frames
        [[nodiscard]] bool IsFreezeFrames() const noexcept;

        // Debug HUD render
        void DebugHudRender(bool show) const;

        // External interface
        void SetPageIndex(std::size_t idx) noexcept;
        std::size_t PageIndex() const noexcept { return _page_index; }

        // Set/Get scroll mode
        void SetScrollMode(ScrollMode mode) noexcept { _mode = mode; }
        ScrollMode GetScrollMode() const noexcept { return _mode; }

        // Set/Get object position (for scrolling reference)
        foundation::math::Vec2& ObjectPos() noexcept { return _object_pos; }
        const foundation::math::Vec2& ObjectPos() const noexcept { return _object_pos; }

        // Set target position (for scrolling reference)
        void SetTargetPos(const foundation::math::Vec2& p) noexcept { _target_pos = p; }

        // Get camera
        Camera& GetCamera() noexcept { return _cam; }
        const Camera& GetCameraConst() const noexcept { return _cam; }

        // ===== Get indirect access components =====
        PageScrollAnimator& Animator() noexcept { return _anim; }
        const PageScrollAnimator& Animator() const noexcept { return _anim; }
        const IScrollRuleProvider* Rules() const noexcept { return &_rules; }
        const apps::systems::view::ViewState& GetView() const noexcept { return _viewState; }

    private:
        void updateViewState_();                                    // Update view state representation
        const PageScroll* activeFixedScrollState_() const noexcept; // Get active fixed scroll state (anim or freeze)
        void drawNeighbors_();                                      // Draw neighboring pages

    private:
        const std::wstring kClassName{ L"ScrollController" };

        const int _tileX{ conf::kTileCountX };
        const int _tileY{ conf::kTileCountY };

        IScrollRuleProvider& _rules;
        MapRenderer2D& _renderer;
        Params _params{};

        // Core state
        foundation::math::Vec2 _view_world{};
        apps::systems::view::ViewState _viewState{};

        ScrollMode _mode{ ScrollMode::PlayerFollow };
        std::size_t _page_index{ 0 };

        foundation::math::Vec2 _object_pos{};
        foundation::math::Vec2 _target_pos{};
        Camera _cam{};
        PageScrollAnimator _anim{};

        // Composition
        ScrollNeighborResolver _neighbor;
        FreeScrollDriver _free;

        ScrollFreezeState _fixed_freeze{};
        FixedScrollDriver _fixed_driver;
    };
}