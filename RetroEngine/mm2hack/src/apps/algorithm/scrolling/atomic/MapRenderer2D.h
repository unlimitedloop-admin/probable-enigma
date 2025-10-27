//==============================================================================
// 
//  Project: mm2hack
//  MapRenderer2D.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include <string>
#include <utility>
#include "apps/supervisor/ResourceManager.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::algorithm::scrolling::atomic
{
    class MapRenderer2D
    {
        using ResourceManager = apps::supervisor::ResourceManager;

    public:
        MapRenderer2D(ResourceManager& res_mgr, std::wstring map_name, std::wstring map_bin_path, int tile_px)
            : _res_mgr(res_mgr), _map_name(std::move(map_name)), _map_bin_path(std::move(map_bin_path)), _tile_px(tile_px)
        {
        }

        void DrawPage(std::size_t page_index, int dx, int dy);
        void DrawAnimation(const PageScroll& pg, std::size_t from_idx, std::size_t to_idx);

    private:
        ResourceManager& _res_mgr;
        std::wstring _map_name;
        std::wstring _map_bin_path;
        int _tile_px{ 16 };
    };
}