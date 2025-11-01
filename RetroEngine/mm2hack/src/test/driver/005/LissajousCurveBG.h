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
#include "apps/rendering/bg/BGTileManager.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/scrolling/effect/BgWobblePass.h"
#include "test/driver/ITestDriver.h"

namespace mm2hack::apps::scenes
{
    class LissajousCurveBG final : public ITestDriver
    {
        using BgWobblePass = systems::scrolling::effect::BgWobblePass;
        using BGTileManager = rendering::bg::BGTileManager;
        using SpriteManager = rendering::sprite::SpriteManager;

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

        SpriteManager::Id _playerId{ static_cast<SpriteManager::Id>(-1) };
        BGTileManager::Id _bgTileId{ static_cast<BGTileManager::Id>(-1) };

        BgWobblePass _bgPass;
        float _deltaTimeSec{ 0.0f };
    };
}