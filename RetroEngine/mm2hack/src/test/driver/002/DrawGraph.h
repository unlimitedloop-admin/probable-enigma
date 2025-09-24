//==============================================================================
// 
//  Project: mm2hack
//  DrawGraph.h
// 
//  This is used in window and graphic size scaling tests.
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/graphics/BGTileManager.h"
#include "apps/graphics/SpriteManager.h"
#include "test/driver/ITestDriver.h"

namespace mm2hack::apps::scenes
{
    class DrawGraph final : public ITestDriver
    {
    public:
        DrawGraph() {}
        ~DrawGraph() {}

        bool Initialize() override;
        bool InitializeResources();
        void Update() override {}
        void RenderWorld() override;
        void RenderOverlay() override {};
        void Finalize() override;

    private:
        const std::wstring kClassName{ L"DrawGraph" };
        const std::wstring kMapName{ L"SAMPLESTAGE1" };

        graphics::SpriteManager::Id _playerId{ static_cast<graphics::SpriteManager::Id>(-1) };
        graphics::BGTileManager::Id _bgTileId{ static_cast<graphics::BGTileManager::Id>(-1) };
    };
}