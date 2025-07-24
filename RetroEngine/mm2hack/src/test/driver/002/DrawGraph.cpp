#include "DrawGraph.h"

#include "apps/deal/GameContext.h"

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
        spriteLoader.SetDivSettings(L"Player", 32, 32, 20, 10);
        if (!spriteLoader.Load(L"Player", L"assets\\sprite\\MegaManAllTiles.png"))
        {
            return false;   // Failed to load the sprite.
        }

        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Load the background tile graph.
        bgTileManager.SetDivSettings(L"SAMPLESTAGE1", 16, 16, 16, 8);
        bgTileManager.Load(L"SAMPLESTAGE1", L"src\\resources\\exams\\bg\\demostage1tiles.png");
        // Load map data
        bgTileManager.LoadMapData(L"src\\resources\\exams\\bg\\SAMPLESTAGE1.bin");

        return true;
    }

    void DrawGraph::Draw()
    {
        using namespace deal;
        auto& spriteDrawer = GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Draw the graph from the resource manager.
        bgTileManager.DrawMap(L"SAMPLESTAGE1", 0, 0);
        spriteDrawer.Use(L"Player", 1, 0, 0);
    }

    void DrawGraph::Finalize()
    {
        using namespace deal;
        GameContext::GetInstance().GetResourceManager().GetSpriteManager().Remove(L"Player");
        GameContext::GetInstance().GetResourceManager().GetBGTileManager().Remove(L"SAMPLESTAGE1");
    }
}