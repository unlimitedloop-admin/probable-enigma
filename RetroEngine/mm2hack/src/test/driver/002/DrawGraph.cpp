#include "pch.h"

#include "DrawGraph.h"

#include "apps/deal/GameContext.h"
#include "apps/graphics/SpriteManager.h"

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
        _playerId = spriteLoader.Load(L"Player",
            L"assets\\sprites\\avatar\\normal\\PLAYER_N0_ALL_PATTERN.png",
            L"assets\\sprites\\avatar\\normal\\PLAYER_N0_ALL_PATTERN.json");
        if (_playerId == graphics::SpriteManager::Id(-1))
        {
            return false;
        }
        spriteLoader.SetGlobalVariant(0); // Default palette variant

        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Load the background tile graph.
        bgTileManager.SetDivSettings(L"SAMPLESTAGE1", 16, 16, 16, 8);
        bgTileManager.Load(L"SAMPLESTAGE1", L"assets\\exams\\bg\\demostage1tiles.png");
        // Load map data.
        bgTileManager.LoadMapData(L"assets\\exams\\bg\\SAMPLESTAGE1.bin");

        return true;
    }

    void DrawGraph::RenderWorld()
    {
        using namespace deal;
        auto& spriteDrawer = GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Draw the graph from the resource manager.
        bgTileManager.DrawMap(L"SAMPLESTAGE1", 0, 0);
        spriteDrawer.UseById(_playerId, 1, 0, 0);
    }

    void DrawGraph::Finalize()
    {
        using namespace deal;
        // Test only?
        if (GameContext::GetInstance().IsShutdown()) return;
        GameContext::GetInstance().GetResourceManager().GetBGTileManager().Remove(L"SAMPLESTAGE1");
    }
}