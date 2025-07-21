#include "DrawGraph.h"

#include "apps/supervisor/ResourceManager.h"

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
        using namespace mm2hack::apps::supervisor;
        auto& spriteLoader = ResourceManager::GetInstance().GetSpriteManager();

        // Load the graph from the resource manager.
        spriteLoader.SetDivSettings(L"Player", 32, 32, 20, 10);
        if (!spriteLoader.Load(L"Player", L"assets\\sprite\\MegaManAllTiles.png"))
        {
            return false;   // Failed to load the sprite.
        }

        return true;
    }

    void DrawGraph::Draw()
    {
        using namespace mm2hack::apps::supervisor;
        auto& spriteDrawer = ResourceManager::GetInstance().GetSpriteManager();

        // Draw the graph from the resource manager.
        spriteDrawer.Use(L"Player", 1, 0, 0);
    }

    void DrawGraph::Finalize()
    {
        using namespace mm2hack::apps::supervisor;
        ResourceManager::GetInstance().GetSpriteManager().Remove(L"Player");
    }
}