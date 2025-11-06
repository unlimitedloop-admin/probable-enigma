//==============================================================================
// 
//  Project: mm2hack
//  MapRenderer2D.h
// 
//  A renderer for laying and manipulating 2D map tiles.
// 
//==============================================================================
#pragma once

#include <string>
#include <utility>
#include "config/SystemConfig.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::resources
{
    class ResourceManager;
}

namespace mm2hack::apps::systems::scrolling::atomic
{
    // Rendering for 2D tile-based map with page scrolling
    class MapRenderer2D
    {
        using ResourceManager = apps::resources::ResourceManager;
        using conf = config::SystemConfig;

    public:
        MapRenderer2D(ResourceManager& res_mgr, std::wstring map_name, std::wstring map_bin_path, int tile_px)
            : _res_mgr(res_mgr), _map_name(std::move(map_name)), _map_bin_path(std::move(map_bin_path)), _tile_px(tile_px)
        {
        }

        // Draw a single page of the map at the specified offset
        void DrawPage(std::size_t page_index, int dx, int dy);
        // Draw animated transition between two pages based on scroll progress
        void DrawAnimation(const PageScroll& pg, std::size_t from_idx, std::size_t to_idx);

    private:
        const std::wstring kClassName = L"MapRenderer2D";

        ResourceManager& _res_mgr;      // Reference to the resource manager
        std::wstring _map_name;         // Name of the map to render
        std::wstring _map_bin_path;     // Path to the map binary file
        int _tile_px{ 16 };             // Tile size in pixels
    };
}