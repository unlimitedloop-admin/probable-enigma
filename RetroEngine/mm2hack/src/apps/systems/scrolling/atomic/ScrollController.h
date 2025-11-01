//==============================================================================
// 
//  Project: mm2hack
//  ScrollController.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdlib>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/ViewState.h"
#include "config/SystemConfig.h"
#include "IScrollRuleProvider.h"
#include "MapRenderer2D.h"
#include "PageScrollAnimator.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    struct Camera
    {
        using Scalar = systems::view::Scalar;
        using conf = config::SystemConfig;

        Scalar x{ 0 }, y{ 0 };
        int vw{ conf::kScreenWidth }, vh{ conf::kScreenHeight };

        static constexpr Scalar kCenterX = conf::kScreenWidth / 2.0;
        static constexpr Scalar kCenterY = conf::kScreenHeight / 2.0;

        static inline bool NearlyZero(Scalar v, Scalar eps = foundation::math::kEps) noexcept { return std::abs(v) <= eps; }
        static inline bool NearlyEqual(Scalar a, Scalar b, Scalar eps = foundation::math::kEps) noexcept { return std::abs(a - b) <= eps; }
    };

    // 差分タイプ（入力・外力など用途を明示）
    struct Diff2
    {
        using Vec2 = foundation::math::Vec2;
        Vec2 delta{};

        [[nodiscard]] bool any(double eps = foundation::math::kEps) const noexcept
        {
            return !Camera::NearlyZero(delta.x, eps) || !Camera::NearlyZero(delta.y, eps);
        }

        void reset() noexcept { delta = Vec2::Zero(); }
    };

    class IScrollPolicy
    {
    public:
        virtual ~IScrollPolicy() = default;
        // 戻り値：ページ切替が起きたら true
        virtual bool Update(const foundation::math::RectF& playerBox, Camera& cam, size_t& currentPageIndex, double dt) = 0;
    };

    // The main video screen policy and management of scrolling in 2D maps
    class ScrollController
    {
        using conf = config::SystemConfig;
        using ViewState = apps::systems::view::ViewState;
        using Vec2 = foundation::math::Vec2;

    public:
        struct Params
        {
            int tile_px{ 16 };  // Tile px
            int view_w{ conf::kScreenWidth };  // Width of the screen
            int view_h{ conf::kScreenHeight };  // Height of the screen
        };

        ScrollController(IScrollRuleProvider& rules, MapRenderer2D& renderer, Params params)
            : _rules(rules), _renderer(renderer), _params(params)
        {
        }

        // フリースクロール／固定アニメを含む総合更新
        void Update(const Vec2& input_delta);

        // 描画
        void Render();

        // 外部公開
        void SetPageIndex(std::size_t idx) noexcept { _page_index = idx; }
        std::size_t PageIndex() const noexcept { return _page_index; }

        Vec2& ObjectPos() noexcept { return _object_pos; }
        const Vec2& ObjectPos() const noexcept { return _object_pos; }

        Camera& GetCamera() noexcept { return _cam; }
        const Camera& GetCameraConst() const noexcept { return _cam; }

        PageScrollAnimator& Animator() noexcept { return _anim; }
        const PageScrollAnimator& Animator() const noexcept { return _anim; }

        // Camera -> ViewState representation
        ViewState GetView() const noexcept
        {
            ViewState v{};
            v.camX = _cam.x;
            v.camY = _cam.y;
            v.viewW = _params.view_w;
            v.viewH = _params.view_h;
            return v;
        }

    private:
        // Sub-update for each axis
        void UpdateAxisX(double remain);
        void UpdateAxisY(double remain);

        // Draw neighboring pages
        void DrawNeighbors();

    private:
        IScrollRuleProvider& _rules;        // Scroll rules
        MapRenderer2D& _renderer;           // Map renderer
        Params _params{};                   // Parameters

        std::size_t _page_index{ 0 };
        Vec2 _object_pos{};
        Camera _cam{};
        PageScrollAnimator _anim{};

        const int _tileX{ conf::kTileCountX };
        const int _tileY{ conf::kTileCountY };
    };
}