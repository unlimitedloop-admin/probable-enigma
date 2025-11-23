//==============================================================================
// 
//  Project: mm2hack
//  DemoStage1Phase.h
// 
//  Phase class for the first demo stage.
// 
//==============================================================================
#pragma once

#include "DemoStage1.h"

#include <memory>
#include <string>
#include "apps/rendering/bg/BGTileMapProvider.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/ITileMapProvider.h"
#include "apps/systems/physics/TileQueryService.h"
#include "apps/systems/scrolling/atomic/MapRenderer2D.h"
#include "apps/systems/scrolling/atomic/ScraperScrollRuleProvider.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::scenes
{
    namespace DemoStage1_
    {
        // Main phase - action stage scene
        class MainPhase : public IDemoStage1Phase
        {
            using AddressScraper            = resources::bg::AddressScraper;
            using MapRenderer2D             = systems::scrolling::atomic::MapRenderer2D;
            using ScrollController          = systems::scrolling::atomic::ScrollController;
            using ScraperScrollRuleProvider = systems::scrolling::atomic::ScraperScrollRuleProvider;

            using BGTileMapProvider         = rendering::bg::BGTileMapProvider;
            using ITerrainProbe             = systems::physics::ITerrainProbe;
            using ITileMapProvider          = systems::physics::ITileMapProvider;
            using TileQueryService          = systems::physics::TileQueryService;

        public:
            explicit MainPhase(DemoStage1& owner) : owner(owner) {}
            void Initialize() override;
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            DemoStage1PhaseId Id() const noexcept override;

        private:
            const std::wstring kClassName{ L"DemoStage1::MainPhase" };
            const int kTilePx{ config::SystemConfig::kTileSize };

            DemoStage1& owner;

            std::unique_ptr<ITileMapProvider> _mapProvider;
            std::unique_ptr<ITerrainProbe>    _terrainProbe;

            std::unique_ptr<MapRenderer2D> _renderer;
            std::unique_ptr<ScraperScrollRuleProvider> _rules;
            std::unique_ptr<ScrollController> _scroll;

            int _page_index_debug{ 0 };
        };
    }
}