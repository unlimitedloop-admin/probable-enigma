//==============================================================================
// 
//  Project: mm2hack
//  DrawGraph.h
// 
//  This is used in window and graphic size scaling tests.
// 
//==============================================================================
#pragma once

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
        graphics::SpriteManager::Id _playerId{ static_cast<graphics::SpriteManager::Id>(-1) };
    };
}