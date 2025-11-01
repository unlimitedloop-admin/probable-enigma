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
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/rendering/bg/BGTileManager.h"
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
        void Update() override;
        void RenderWorld() override;
        void RenderOverlay() override {};
        void Finalize() override;

    private:
        const std::wstring kClassName{ L"DrawGraph" };
        const std::wstring kMapName{ L"SAMPLESTAGE1" };
        const std::wstring_view kStageMapBinary{ L"assets\\_exams\\bg\\SAMPLESTAGE1.bin" };

        const int fadeDurationFrames{ 16 };

        rendering::sprite::SpriteManager::Id _playerId{ static_cast<rendering::sprite::SpriteManager::Id>(-1) };
        rendering::bg::BGTileManager::Id _bgTileId{ static_cast<rendering::bg::BGTileManager::Id>(-1) };
    };
}