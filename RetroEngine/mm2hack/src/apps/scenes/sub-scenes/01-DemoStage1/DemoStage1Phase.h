//==============================================================================
// 
//  Project: mm2hack
//  DemoStage1Phase.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "DemoStage1.h"

#include <memory>
#include <string>
#include "apps/algorithm/scrolling/atomic/MapRenderer2D.h"
#include "apps/algorithm/scrolling/atomic/ScraperScrollRuleProvider.h"
#include "apps/algorithm/scrolling/atomic/ScrollController.h"
#include "apps/graphics/bg/AddressScraper.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        // Main phase - action stage scene
        class MainPhase : public IDemoStage1Phase
        {
            using AddressScraper = graphics::bg::AddressScraper;
            using MapRenderer2D = algorithm::scrolling::atomic::MapRenderer2D;
            using ScrollController = algorithm::scrolling::atomic::ScrollController;
            using ScraperScrollRuleProvider = algorithm::scrolling::atomic::ScraperScrollRuleProvider;

        public:
            explicit MainPhase(DemoStage1& owner) : owner(owner) {}
            void Initialize() override;
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            DemoStage1PhaseId Id() const noexcept override;

        private:
            const std::wstring kClassName{ L"DemoStage1::MainPhase" };
            const int kTilePx{ config::SystemConfig::kTileSizeWidth };

            DemoStage1& owner;

            std::unique_ptr<MapRenderer2D> _renderer;
            std::unique_ptr<ScraperScrollRuleProvider> _rules;
            std::unique_ptr<ScrollController> _scroll;

            int _page_index_debug{ 0 };
        };
    }
}