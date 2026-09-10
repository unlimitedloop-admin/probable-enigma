//==============================================================================
// 
//  Project: mm2hack
//  MapRenderer2D.h
// 
//  A renderer for laying and manipulating 2D map tiles.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include <utility>

#include "config/SystemConfig.h"
#include "ScrollTypes.h"

namespace mm2hack::apps::resources
{
    class ResourceManager;

    namespace bg
    {
        class IMapPageSource;
    }
}

namespace mm2hack::apps::systems::scrolling::atomic
{
    // Rendering for 2D tile-based map with page scrolling
    class MapRenderer2D
    {
        using ResourceManager = apps::resources::ResourceManager;
        using IMapPageSource = apps::resources::bg::IMapPageSource;
        using conf = config::SystemConfig;

    public:
        MapRenderer2D(
            ResourceManager& res_mgr,
            std::wstring map_name,
            std::shared_ptr<const IMapPageSource> page_source,
            int tile_px
        )
            : _res_mgr(res_mgr),
              _map_name(std::move(map_name)),
              _page_source(std::move(page_source)),
              _tile_px(tile_px)
        {
        }

        // Draw a single page of the map at the specified offset
        void DrawPage(std::size_t page_index, int dx, int dy);
        // Draw animated transition between two pages based on scroll progress
        void DrawAnimation(const PageScroll& pg, std::size_t from_idx, std::size_t to_idx);

    private:
        const std::wstring kClassName{ L"MapRenderer2D" };

        ResourceManager& _res_mgr;      // Reference to the resource manager
        std::wstring _map_name;         // Name of the map to render
        std::shared_ptr<const IMapPageSource> _page_source; // Shared in-memory map page source
        int _tile_px{ 16 };             // Tile size in pixels
    };
}
