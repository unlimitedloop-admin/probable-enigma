#include "pch.h"

#include "DrawGraph.h"

#include "apps/runtime/GameContext.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/rendering/bg/BGTileManager.h"
#include "config/GameAssets.h"

namespace mm2hack::apps::scenes
{
    bool DrawGraph::Initialize()
    {
        return InitializeResources();
    }

    bool DrawGraph::InitializeResources()
    {
        using namespace config;
        using namespace runtime;
        auto& resource = GameContext::GetInstance().GetResourceManager();
        // Load the graph from the resource manager.
        auto& spriteLoader = resource.GetSpriteManager();
        _playerId = spriteLoader.Load(L"Player", MM2H_GRAPHICS(Player), MM2H_GRAPHPROPS(Player));
        if (_playerId == rendering::sprite::SpriteManager::Id(-1))
        {
            return false;
        }
        const int vmax = spriteLoader.VariantCountById(_playerId);
        spriteLoader.SetGlobalVariant(vmax);
        resource.FadeInSprite(fadeDurationFrames);

        // Load the background tile graph.
        auto& bgTileManager = resource.GetBGTileManager();
        _bgTileId = bgTileManager.LoadTileset(kMapName, MM2H_GRAPHICS(SampleStage), MM2H_GRAPHPROPS(SampleStage));
        if (_bgTileId == rendering::bg::BGTileManager::Id(-1))
        {
            return false;
        }
        bgTileManager.SetMapSize(SystemConfig::kTileCountX, SystemConfig::kTileCountY);     // 16x15 tiles
        // Load map data.
        bgTileManager.LoadMapBinary(kStageMapBinary, 0x10);
        const int bvmax = bgTileManager.VariantCountById(_bgTileId);
        bgTileManager.SetGlobalVariant(bvmax);
        resource.FadeInBG(fadeDurationFrames);

        return true;
    }

    void DrawGraph::Update()
    {
        using namespace runtime;
        auto& resource = GameContext::GetInstance().GetResourceManager();
        resource.UpdateEffects();   // Update fade effects
    }

    void DrawGraph::RenderWorld()
    {
        using namespace config;
        using namespace runtime;
        auto& spriteDrawer = GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();

        // Draw the graph from the resource manager.
        bgTileManager.DrawMapByName(kMapName, SystemConfig::kTileSizeWidth, SystemConfig::kTileSizeHeight, 0, 0);
        spriteDrawer.UseById(_playerId, 1, 0, 0);
    }

    void DrawGraph::Finalize()
    {
        // No special finalization needed
    }
}