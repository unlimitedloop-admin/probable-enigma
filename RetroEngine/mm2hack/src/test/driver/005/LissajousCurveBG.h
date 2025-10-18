//==============================================================================
// 
//  Project: mm2hack
//  LissajousCurveBG.h
// 
//  A test of Summer Carnival '92 Recca-style multi-layered BG scrolling script.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/algorithm/scrolling/effect/BgWobblePass.h"
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/graphics/sprite/SpriteManager.h"
#include "test/driver/ITestDriver.h"

namespace mm2hack::apps::scenes
{
    class LissajousCurveBG final : public ITestDriver
    {
    public:
        LissajousCurveBG() {}
        ~LissajousCurveBG() {}

        bool Initialize() override;
        bool InitializeResources();
        void Update() override;
        void RenderWorld() override;
        void RenderOverlay() override {};
        void Finalize() override;

    private:
        const std::wstring kClassName{ L"LissajousCurveBG" };

        const int fadeDurationFrames{ 16 };

        graphics::sprite::SpriteManager::Id _playerId{ static_cast<graphics::sprite::SpriteManager::Id>(-1) };
        graphics::bg::BGTileManager::Id _bgTileId{ static_cast<graphics::bg::BGTileManager::Id>(-1) };

        apps::algorithm::scrolling::effect::BgWobblePass _bgPass;
        float _deltaTimeSec{ 0.0f };
    };
}