//==============================================================================
// 
//  Project: mm2hack
//  Scroll.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/graphics/bg/RoomGraphAdapter.h"
#include "apps/mod/CoordinateTypes.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    struct Camera
    {
        apps::mod::Scalar x{ 0 }, y{ 0 };
        int vw{ 256 }, vh{ 240 };
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

    class ScrollController
    {
        using RoomGraphAdapter = graphics::bg::RoomGraphAdapter;
        using BGTileManager = graphics::bg::BGTileManager;

    public:
        ScrollController(Camera cam,
            std::unique_ptr<IScrollPolicy> policy,
            RoomGraphAdapter& graph,
            BGTileManager& bg,
            const std::wstring& tilesetName,
            const std::wstring& mapBinPath)
            : _cam(cam), _policy(std::move(policy)), _graph(graph), _bg(bg),
            _tileset(tilesetName), _mapPath(mapBinPath)
        {
        }

        // pageIndex は「現在のページ」を保持（roomNo→pageIndexは初期化時に解決）
        void Initialize(size_t pageIndex, int tilePx)
        {
            _loadedPage = SIZE_MAX;
            _tilePx = tilePx;
            _currentPage = pageIndex;
            _bg.SetMapSize(16, 15);
            reloadIfNeeded(true);
        }

        // 1フレーム更新。必要ならページを切替えて BG を再ロード
        void Update(const apps::mod::RectF& playerBox, double dt)
        {
            if (_policy->Update(playerBox, _cam, _currentPage, dt))
            {
                reloadIfNeeded(true);
            }
        }

        // 衝突系へ“カメラオフセット”を供給
        template <class CollisionService>
        void FeedTo(CollisionService& col) const
        {
            col.SetWorldOffset(static_cast<float>(_cam.x), static_cast<float>(_cam.y));
        }

        const Camera& GetCamera() const noexcept { return _cam; }

    private:
        void reloadIfNeeded(bool force)
        {
            if (force || _loadedPage != _currentPage)
            {
                // AddressScraper を使って該当ページの 0x10 オフセットへ
                const int offset = static_cast<int>(_currentPage * 0x100 + 0x10);
                _bg.LoadMapBinary(_mapPath, offset);
                _loadedPage = _currentPage;
                // ★ $0B 属性タイプに応じた属性テーブル適用をここで（前回の ApplyAttrByPage 相当）
            }
        }

    private:
        Camera _cam;
        std::unique_ptr<IScrollPolicy> _policy;
        RoomGraphAdapter& _graph;
        BGTileManager& _bg;
        std::wstring _tileset, _mapPath;

        size_t _currentPage{ 0 }, _loadedPage{ SIZE_MAX };
        int _tilePx{ 8 };
    };
}