#include "pch.h"

#include "SeamlessBGRenderer.h"

#include <cstdint>
#include "apps/graphics/bg/BGTileManager.h"
#include "ScrollController.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    // 内部ユーティリティ: 指定矩形のタイルブロックを描画する汎用関数
    namespace {
        template<typename TileAt>
        inline void DrawTileBlock(const graphics::bg::BGTileManager& bg,
                                  graphics::bg::BGTileManager::Id tileset,
                                  TileAt tileAt,
                                  int w, int h,
                                  int dstX, int dstY,
                                  int tilePx) noexcept
        {
            for (int ty = 0; ty < h; ++ty)
            {
                for (int tx = 0; tx < w; ++tx)
                {
                    const int id = static_cast<int>(tileAt(tx, ty));
                    bg.DrawTileById(tileset, id, dstX + tx * tilePx, dstY + ty * tilePx);
                }
            }
        }
    }

    SeamlessBGRenderer::SeamlessBGRenderer(BGTileManager& bg, AddressScraper& scraper, BGTileManager::Id tilesetId, int tilePx)
        : _bg(bg), _cache(scraper), _tileset(tilesetId), _ts(tilePx)
    {
    }

    void SeamlessBGRenderer::Draw(const Camera& cam, size_t currentPageIndex)
    {
        _cache.BuildAround(currentPageIndex);

        const int vw = cam.vw, vh = cam.vh;
        const int pageW = 16 * _ts;
        const int pageH = 15 * _ts;

        // 現在ページの画面上への描画オフセット（カメラは世界→画面のマイナス）
        const int ox = -static_cast<int>(cam.x);
        const int oy = -static_cast<int>(cam.y);

        // どの隣接が必要か（はみ出し量で決定）
        const bool needRight = (ox + pageW) < vw;
        const bool needDown = (oy + pageH) < vh;
        const bool needLeft = (ox > 0);
        const bool needUp = (oy > 0);

        // 1) 現在ページ
        drawPage_(currentPageIndex, ox, oy);

        // 2) 横/縦
        if (needRight) if (auto p = _cache.Right(currentPageIndex)) drawPage_(*p, ox + pageW, oy);
        if (needDown)  if (auto p = _cache.Down(currentPageIndex)) drawPage_(*p, ox, oy + pageH);
        if (needLeft)  if (auto p = _cache.Left(currentPageIndex)) drawPage_(*p, ox - pageW, oy);
        if (needUp)    if (auto p = _cache.Up(currentPageIndex)) drawPage_(*p, ox, oy - pageH);

        // 3) 斜め（必要なときだけ）
        if (needRight && needDown) if (auto p = _cache.RightDown(currentPageIndex)) drawPage_(*p, ox + pageW, oy + pageH);
        if (needLeft && needDown) if (auto p = _cache.LeftDown(currentPageIndex)) drawPage_(*p, ox - pageW, oy + pageH);
        if (needRight && needUp)   if (auto p = _cache.RightUp(currentPageIndex)) drawPage_(*p, ox + pageW, oy - pageH);
        if (needLeft && needUp)   if (auto p = _cache.LeftUp(currentPageIndex)) drawPage_(*p, ox - pageW, oy - pageH);
    }

    void SeamlessBGRenderer::drawPage_(size_t pageIndex, int dstX, int dstY)
    {
        // 触っておく（無意味アクセスでもOK、先読み等の副作用を期待）
        const auto& tiles = _cache.Tile(pageIndex, 0, 0);

        // ヘルパーに委譲して描画（将来的に BGTileManager 側へ移動して共通化可）
        DrawTileBlock(_bg, _tileset,
            [this, pageIndex](int tx, int ty) -> std::uint8_t {
                return _cache.Tile(pageIndex, tx, ty);
            },
            16, 15,
            dstX, dstY,
            _ts);
    }
}