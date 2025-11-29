//==============================================================================
// 
//  Project: mm2hack
//  ScrollController.h
// 
//  Manages scrolling behavior in 2D map systems.
// 
//==============================================================================
#pragma once

#include <cstdlib>
#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/ViewState.h"
#include "config/SystemConfig.h"
#include "IScrollRuleProvider.h"
#include "MapRenderer2D.h"
#include "PageScrollAnimator.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    using foundation::math::kEps;

    // View camera representation
    struct Camera
    {
        using Scalar = systems::view::Scalar;
        using conf = config::SystemConfig;

        Scalar x{ 0 }, y{ 0 };
        int vw{ conf::kScreenWidth }, vh{ conf::kScreenHeight };

        static constexpr Scalar kCenterX = conf::kScreenWidth / 2.0;
        static constexpr Scalar kCenterY = conf::kScreenHeight / 2.0;

        static inline bool NearlyZero(Scalar v, Scalar eps = kEps) noexcept { return std::abs(v) <= eps; }
        static inline bool NearlyEqual(Scalar a, Scalar b, Scalar eps = kEps) noexcept { return std::abs(a - b) <= eps; }
    };

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

    // Interface for scroll policy
    class IScrollPolicy
    {
    public:
        virtual ~IScrollPolicy() = default;
        virtual bool Update(const foundation::math::RectF& playerBox, Camera& cam, size_t& currentPageIndex, double dt) = 0;
    };

    // The main video screen policy and management of scrolling in 2D maps
    class ScrollController
    {
        using ViewState = apps::systems::view::ViewState;
        using conf      = config::SystemConfig;
        using Vec2      = foundation::math::Vec2;

    public:
        struct Params
        {
            int tile_px{ conf::kTileSize };     // Tile px
            int view_w{ conf::kScreenWidth };   // Width of the screen
            int view_h{ conf::kScreenHeight };  // Height of the screen
        };

        ScrollController(IScrollRuleProvider& rules, MapRenderer2D& renderer, Params params)
            : _rules(rules), _renderer(renderer), _params(params)
        {
        }

        // Free scroll / fixed animation comprehensive update
        void Update(const Vec2& input_delta);

        // Rendering
        void Render();
        // Set scroll mode
        void SetMode(ScrollKind m) noexcept { _mode = m; }

        void SyncWithObjectCenter(const Vec2& object_center, bool has_adj_x, bool has_adj_y, const Vec2& screen_px, const Vec2& map_px, ViewState& out_view);

        // External interface
        void SetPageIndex(std::size_t idx) noexcept { _page_index = idx; }
        std::size_t PageIndex() const noexcept { return _page_index; }

        Vec2& ObjectPos() noexcept { return _object_pos; }
        const Vec2& ObjectPos() const noexcept { return _object_pos; }

        Camera& GetCamera() noexcept { return _cam; }
        const Camera& GetCameraConst() const noexcept { return _cam; }

        PageScrollAnimator& Animator() noexcept { return _anim; }
        const PageScrollAnimator& Animator() const noexcept { return _anim; }

        // Camera -> ViewState representation
        const ViewState& GetView() const noexcept { return _viewState; }

    private:
        void updateAxisX_(double remain);   // Sub-update for each axis
        void updateAxisY_(double remain);   // Sub-update for each axis

        void drawNeighbors_();              // Draw neighboring pages
        void updateViewState_();            // Update ViewState representation

    private:
        const std::wstring kClassName = L"ScrollController";

        IScrollRuleProvider& _rules;                // Scroll rules
        MapRenderer2D& _renderer;                   // Map renderer
        Params _params{};                           // Parameters

        ScrollKind _mode{ ScrollKind::None };       // Current scroll mode
        std::size_t _page_index{ 0 };               // Current page index
        Vec2 _object_pos{};                         // Object position
        Camera _cam{};                              // Camera
        PageScrollAnimator _anim{};                 // Page scroll animator

        const int _tileX{ conf::kTileCountX };      // Tile count X
        const int _tileY{ conf::kTileCountY };      // Tile count Y

        ViewState _viewState{};                     // View state representation
    };
}