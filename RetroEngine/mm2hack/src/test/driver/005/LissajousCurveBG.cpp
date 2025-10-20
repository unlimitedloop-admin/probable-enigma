#include "pch.h"

#include "LissajousCurveBG.h"

#include <numbers>
#include "apps/deal/GameContext.h"
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/graphics/sprite/SpriteManager.h"
#include "config/GameAssets.h"
#include "core/winapi/WindowManager.h"

namespace mm2hack::apps::scenes
{
    bool LissajousCurveBG::Initialize()
    {
        return InitializeResources();
    }

    bool LissajousCurveBG::InitializeResources()
    {
        using namespace config;
        using namespace deal;
        using conf = SystemConfig;

        auto& resource = GameContext::GetInstance().GetResourceManager();
        // Load the graph from the resource manager.
        auto& spriteLoader = resource.GetSpriteManager();
        _playerId = spriteLoader.Load(L"Player", MM2H_GRAPHICS(Player), MM2H_GRAPHPROPS(Player));
        if (_playerId == graphics::sprite::SpriteManager::Id(-1))
        {
            return false;
        }
        const int vmax = spriteLoader.VariantCountById(_playerId);
        spriteLoader.SetGlobalVariant(vmax);
        resource.FadeInSprite(fadeDurationFrames);

        // Load the background tile graph.
        auto& bgTileManager = resource.GetBGTileManager();
        _bgTileId = bgTileManager.LoadTileset(L"SampleNoise", MM2H_GRAPHICS(SampleNoise), MM2H_GRAPHPROPS(SampleNoise));
        if (_bgTileId == graphics::bg::BGTileManager::Id(-1))
        {
            return false;
        }
        const int bvmax = bgTileManager.VariantCountById(_bgTileId);
        bgTileManager.SetGlobalVariant(bvmax);
        resource.FadeInBG(fadeDurationFrames);

        _bgPass.Initialize(conf::kScreenWidth, conf::kScreenHeight, /*stripe_h*/ 2);
        _bgPass.SetParams(
            /*amp_x_px*/ 27.0f,
            /*amp_y_px*/ 24.0f,
            /*freq_y  */ 0.0f,
            /*speed   */ 2.5f,
            /*phase0  */ 0.0f,
            /*edge    */ 0.2f
        );
        _bgPass.SetRasterBanding(
            /*band_h_px     */ 16,
            /*phase_step_rad*/ std::numbers::pi_v<float> / 19.9f,
            /*bottom_to_top */ true
        );

        return true;
    }

    void LissajousCurveBG::Update()
    {
        using namespace deal;
        auto& resource = GameContext::GetInstance().GetResourceManager();
        resource.UpdateEffects();   // Update fade effects

        // Update delta time
        auto& time = GameContext::GetInstance().Time();
        _deltaTimeSec = static_cast<float>(time.DeltaSeconds());
        _bgPass.Update(_deltaTimeSec);
    }

    void LissajousCurveBG::RenderWorld()
    {
        using namespace config;
        using namespace deal;
        auto& spriteDrawer = GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        auto& bgTileManager = GameContext::GetInstance().GetResourceManager().GetBGTileManager();
        auto& wm = core::winapi::WindowManager::GetInstance();
        const int screenW = SystemConfig::kScreenWidth;
        const int screenH = SystemConfig::kScreenHeight;
        const int screenHandle = wm.GetScreenHandle();

        _bgPass.Begin();
        bgTileManager.DrawTileById(_bgTileId, 0, 0, 0);
        _bgPass.EndAndCompositeToRT(screenHandle, screenW, screenH, _deltaTimeSec);
        spriteDrawer.UseById(_playerId, 1, 120, 208);
    }

    void LissajousCurveBG::Finalize()
    {
        // No special finalization needed
    }
}