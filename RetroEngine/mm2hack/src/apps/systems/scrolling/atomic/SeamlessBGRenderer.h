//==============================================================================
// 
//  Project: mm2hack
//  SeamlessBGRenderer.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "apps/rendering/bg/BGTileManager.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/resources/bg/MapPageCache.h"
#include "ScrollController.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // シームレス背景描画支援クラス
    class SeamlessBGRenderer
    {
        using AddressScraper = apps::resources::bg::AddressScraper;
        using MapPageCache = apps::resources::bg::MapPageCache;
        using BGTileManager = rendering::bg::BGTileManager;

    public:
        SeamlessBGRenderer(BGTileManager& bg, AddressScraper& scraper, BGTileManager::Id tilesetId, int tilePx);

        // currentPage を左上原点(0,0) とするページ座標系で、
        // カメラオフセット cam.x/y を用い、画面に必要なページを最大4枚まで描く
        void Draw(const Camera& cam, size_t currentPageIndex);

    private:
        void drawPage_(size_t pageIndex, int dstX, int dstY);

        BGTileManager& _bg;
        MapPageCache _cache;
        BGTileManager::Id _tileset;
        int _ts;
    };
}