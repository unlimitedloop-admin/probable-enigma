//==============================================================================
// 
//  Project: mm2hack
//  SeamlessBGRenderer.h
// 
//  An NES-style pseudo-viewer with a background scrolling system.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/resources/bg/MapPageCache.h"
#include "ScrollController.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // Seamless background rendering support class
    class SeamlessBGRenderer
    {
        using AddressScraper = apps::resources::bg::AddressScraper;
        using MapPageCache = apps::resources::bg::MapPageCache;
        using BGTileManager = rendering::bg::BGTileManager;

    public:
        SeamlessBGRenderer(BGTileManager& bg, AddressScraper& scraper, BGTileManager::Id tilesetId, int tilePx);
        ~SeamlessBGRenderer() = default;

        // Draw up to 4 pages on the screen using the camera offset cam.x/y
        void Draw(const Camera& cam, size_t currentPageIndex);

    private:
        void drawPage_(size_t pageIndex, int dstX, int dstY);   // Draw a single page at the specified screen position

    private:
        const std::wstring kClassName = L"SeamlessBGRenderer";

        BGTileManager& _bg;
        MapPageCache _cache;
        BGTileManager::Id _tileset;
        int _ts;
    };
}