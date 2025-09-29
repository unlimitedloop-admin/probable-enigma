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
#include <string_view>
#include "apps/graphics/bg/BGTileManager.h"
#include "apps/graphics/sprite/SpriteManager.h"
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
        const std::wstring_view kStageMapBinary{ L"assets\\_exams\\bg\\SAMPLESTAGE1.bin" };

        graphics::sprite::SpriteManager::Id _playerId{ static_cast<graphics::sprite::SpriteManager::Id>(-1) };
        graphics::bg::BGTileManager::Id _bgTileId{ static_cast<graphics::bg::BGTileManager::Id>(-1) };
    };
}