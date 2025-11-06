#include "pch.h"

#include "SeamlessBGRenderer.h"

#include "apps/rendering/bg/BGTileManager.h"
#include "ScrollController.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // Internal utility: Generic function to draw a tile block in the specified rectangle
    namespace {
        template<typename TileAt>
        inline void DrawTileBlock(const rendering::bg::BGTileManager& bg,
                                  rendering::bg::BGTileManager::Id tileset,
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
        using conf = config::SystemConfig;

        _cache.BuildAround(currentPageIndex);

        const int vw = cam.vw, vh = cam.vh;
        const int pageW = conf::kTileCountX * _ts;
        const int pageH = conf::kTileCountY * _ts;

        // Current page's screen drawing offset (camera is world -> screen negative)
        const int ox = -static_cast<int>(cam.x);
        const int oy = -static_cast<int>(cam.y);

        // Determine which adjacent pages are needed (based on overflow)
        const bool needRight = (ox + pageW) < vw;
        const bool needDown = (oy + pageH) < vh;
        const bool needLeft = (ox > 0);
        const bool needUp = (oy > 0);

        // 1) Current page
        drawPage_(currentPageIndex, ox, oy);

        // 2) Horizontal/Vertical
        if (needRight) if (auto p = _cache.Right(currentPageIndex)) drawPage_(*p, ox + pageW, oy);
        if (needDown)  if (auto p = _cache.Down(currentPageIndex)) drawPage_(*p, ox, oy + pageH);
        if (needLeft)  if (auto p = _cache.Left(currentPageIndex)) drawPage_(*p, ox - pageW, oy);
        if (needUp)    if (auto p = _cache.Up(currentPageIndex)) drawPage_(*p, ox, oy - pageH);

        // 3) Diagonal (only if needed)
        if (needRight && needDown) if (auto p = _cache.RightDown(currentPageIndex)) drawPage_(*p, ox + pageW, oy + pageH);
        if (needLeft && needDown) if (auto p = _cache.LeftDown(currentPageIndex)) drawPage_(*p, ox - pageW, oy + pageH);
        if (needRight && needUp)   if (auto p = _cache.RightUp(currentPageIndex)) drawPage_(*p, ox + pageW, oy - pageH);
        if (needLeft && needUp)   if (auto p = _cache.LeftUp(currentPageIndex)) drawPage_(*p, ox - pageW, oy - pageH);
    }

    void SeamlessBGRenderer::drawPage_(size_t pageIndex, int dstX, int dstY)
    {
        // Touch the tiles (meaningless access is OK, expecting side effects like prefetching).
        const auto& tiles = _cache.Tile(pageIndex, 0, 0);

        // Delegate to helper for drawing (could be moved to BGTileManager side for commonality in the future).
        DrawTileBlock(_bg, _tileset,
            [this, pageIndex](int tx, int ty) -> std::uint8_t {
                return _cache.Tile(pageIndex, tx, ty);
            },
            16, 15,
            dstX, dstY,
            _ts);
    }
}