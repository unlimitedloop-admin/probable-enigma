#include "pch.h"

#include "MapRenderer2D.h"

#include "apps/resources/ResourceManager.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    static inline int PageBase(int page_index)
    {
        using conf = config::SystemConfig;
        return page_index * conf::kMapBinaryUnitPageSize + conf::kMapBinaryHeaderSize;
    }

    void MapRenderer2D::DrawPage(std::size_t page_index, int dx, int dy)
    {
        auto& bg = _res_mgr.GetBGTileManager();
        bg.LoadMapBinary(_map_bin_path, PageBase(static_cast<int>(page_index)));
        bg.DrawMapByName(_map_name, conf::kTileSizeWidth, conf::kTileSizeHeight, dx, dy);
    }

    void MapRenderer2D::DrawAnimation(const PageScroll& pg, std::size_t from_idx, std::size_t to_idx)
    {
        using PageDir = PageScroll::Dir;

        const int page_w = _tile_px * conf::kTileCountX;
        const int page_h = _tile_px * conf::kTileCountY;
        const int prog = static_cast<int>(pg.progress);

        switch (pg.dir)
        {
        case PageDir::Right:
            DrawPage(from_idx, -prog, 0);
            DrawPage(to_idx, page_w - prog, 0);
            break;

        case PageDir::Left:
            DrawPage(from_idx, prog, 0);
            DrawPage(to_idx, -page_w + prog, 0);
            break;

        case PageDir::Down:
            DrawPage(from_idx, 0, -prog);
            DrawPage(to_idx, 0, page_h - prog);
            break;

        case PageDir::Up:
            DrawPage(from_idx, 0, prog);
            DrawPage(to_idx, 0, -page_h + prog);
            break;

        default:
            break;
        }
    }
}