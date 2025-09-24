#include "pch.h"

#include "DrawGraph.h"

#include "apps/deal/GameContext.h"
#include "apps/graphics/BGTileManager.h"
#include "apps/graphics/SpriteManager.h"
#include "config/GameAssets.h"

namespace mm2hack::apps::scenes
{
    bool DrawGraph::Initialize()
    {
        if (!InitializeResources())
        {
            return false;
        }

        return true;
    }

    bool DrawGraph::InitializeResources()
    {
        using namespace deal;
        auto& spriteLoader = GameContext::GetInstance().GetResourceManager().GetSpriteManager();

        // Load the graph from the resource manager.
        _playerId = spriteLoader.Load(L"Player", MM2H_GRAPHICS(Player), MM2H_PROPERTIES(Player));
        if (_playerId == graphics::SpriteManager::Id(-1))
        {
            return false;
        }
        spriteLoader.SetGlobalVariant(0); // Default palette variant

        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Load the background tile graph.
        _bgTileId = bgTileManager.LoadTileset(kMapName, MM2H_GRAPHICS(SampleStage), MM2H_PROPERTIES(SampleStage));
        if (_bgTileId == graphics::BGTileManager::Id(-1))
        {
            return false;
        }
        bgTileManager.SetMapSize(16, 15); // 16x15 tiles
        // Load map data.
        bgTileManager.LoadMapBinary(L"assets\\exams\\bg\\SAMPLESTAGE1.bin", 0x10);
        bgTileManager.SetGlobalVariant(0); // Default palette variant
        return true;
    }

    void DrawGraph::RenderWorld()
    {
        using namespace deal;
        auto& spriteDrawer = GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Draw the graph from the resource manager.
        bgTileManager.DrawMapByName(kMapName, 16, 16, 0, 0);
        spriteDrawer.UseById(_playerId, 1, 0, 0);
    }

    void DrawGraph::Finalize()
    {
        using namespace deal;
        if (GameContext::GetInstance().IsShutdown()) return;
    }
}