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
#include "apps/mod/CoordinateTypes.h"
#include "IScrollRuleProvider.h"
#include "MapRenderer2D.h"
#include "PageScrollAnimator.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    using Scalar = double;

    struct Camera
    {
        Scalar x{ 0 }, y{ 0 };
        int vw{ 256 }, vh{ 240 };

        static constexpr Scalar kCenterX = 128.0;   // 256/2
        static constexpr Scalar kCenterY = 120.0;   // 240/2

        static inline bool NearlyZero(Scalar v, Scalar eps = mod::kEps) noexcept { return std::abs(v) <= eps; }
        static inline bool NearlyEqual(Scalar a, Scalar b, Scalar eps = mod::kEps) noexcept { return std::abs(a - b) <= eps; }
    };

    // 差分タイプ（入力・外力など用途を明示）
    struct Diff2
    {
        mod::Vec2 delta{};

        [[nodiscard]] bool any(double eps = mod::kEps) const noexcept
        {
            return !Camera::NearlyZero(delta.x, eps) || !Camera::NearlyZero(delta.y, eps);
        }

        void reset() noexcept { delta = mod::Vec2::Zero(); }
    };

    class IScrollPolicy
    {
    public:
        virtual ~IScrollPolicy() = default;
        // 戻り値：ページ切替が起きたら true
        virtual bool Update(const apps::mod::RectF& playerBox,
            Camera& cam,
            size_t& currentPageIndex,
            double dt) = 0;
    };

    //class ScrollController
    //{
    //    using RoomGraphAdapter = graphics::bg::RoomGraphAdapter;
    //    using BGTileManager = graphics::bg::BGTileManager;

    //public:
    //    ScrollController(Camera cam,
    //        std::unique_ptr<IScrollPolicy> policy,
    //        RoomGraphAdapter& graph,
    //        BGTileManager& bg,
    //        const std::wstring& tilesetName,
    //        const std::wstring& mapBinPath)
    //        : _cam(cam), _policy(std::move(policy)), _graph(graph), _bg(bg),
    //        _tileset(tilesetName), _mapPath(mapBinPath)
    //    {
    //    }

    //    // pageIndex は「現在のページ」を保持（roomNo→pageIndexは初期化時に解決）
    //    void Initialize(size_t pageIndex, int tilePx)
    //    {
    //        _loadedPage = SIZE_MAX;
    //        _tilePx = tilePx;
    //        _currentPage = pageIndex;
    //        _bg.SetMapSize(16, 15);
    //        reloadIfNeeded(true);
    //    }

    //    // 1フレーム更新。必要ならページを切替えて BG を再ロード
    //    void Update(const apps::mod::RectF& playerBox, double dt)
    //    {
    //        if (_policy->Update(playerBox, _cam, _currentPage, dt))
    //        {
    //            reloadIfNeeded(true);
    //        }
    //    }

    //    // 衝突系へ“カメラオフセット”を供給
    //    template <class CollisionService>
    //    void FeedTo(CollisionService& col) const
    //    {
    //        col.SetWorldOffset(static_cast<float>(_cam.x), static_cast<float>(_cam.y));
    //    }

    //    const Camera& GetCamera() const noexcept { return _cam; }

    //private:
    //    void reloadIfNeeded(bool force)
    //    {
    //        if (force || _loadedPage != _currentPage)
    //        {
    //            // AddressScraper を使って該当ページの 0x10 オフセットへ
    //            const int offset = static_cast<int>(_currentPage * 0x100 + 0x10);
    //            _bg.LoadMapBinary(_mapPath, offset);
    //            _loadedPage = _currentPage;
    //            // ★ $0B 属性タイプに応じた属性テーブル適用をここで（前回の ApplyAttrByPage 相当）
    //        }
    //    }

    //private:
    //    Camera _cam;
    //    std::unique_ptr<IScrollPolicy> _policy;
    //    RoomGraphAdapter& _graph;
    //    BGTileManager& _bg;
    //    std::wstring _tileset, _mapPath;

    //    size_t _currentPage{ 0 }, _loadedPage{ SIZE_MAX };
    //    int _tilePx{ 8 };
    //};

    // REVIEW: 上記のクラスを改訂
    // 8方向スクロールの意思決定と状態管理のコア
    class ScrollController
    {
    public:
        struct Params
        {
            int tile_px{ 16 };          // 1タイル px
            int view_w{ 256 };         // 画面幅
            int view_h{ 240 };         // 画面高
        };

        ScrollController(IScrollRuleProvider& rules,
            MapRenderer2D& renderer,
            Params params)
            : _rules(rules), _renderer(renderer), _params(params)
        {
        }

        // フリースクロール／固定アニメを含む総合更新
        void Update(const mod::Vec2& input_delta);

        // 描画
        void Render();

        // 外部公開
        void SetPageIndex(std::size_t idx) noexcept { _page_index = idx; }
        std::size_t PageIndex() const noexcept { return _page_index; }

        mod::Vec2& ObjectPos() noexcept { return _object_pos; }
        const mod::Vec2& ObjectPos() const noexcept { return _object_pos; }

        Camera& GetCamera() noexcept { return _cam; }
        const Camera& GetCameraConst() const noexcept { return _cam; }

        PageScrollAnimator& Animator() noexcept { return _anim; }
        const PageScrollAnimator& Animator() const noexcept { return _anim; }

    private:
        // 補助
        void UpdateAxisX(double remain);
        void UpdateAxisY(double remain);

        // 近傍ページ描画（自由スクロール中）
        void DrawNeighbors();

    private:
        IScrollRuleProvider& _rules;
        MapRenderer2D& _renderer;
        Params _params{};

        std::size_t _page_index{ 0 };
        mod::Vec2 _object_pos{};
        Camera _cam{};
        PageScrollAnimator _anim{};
    };
}