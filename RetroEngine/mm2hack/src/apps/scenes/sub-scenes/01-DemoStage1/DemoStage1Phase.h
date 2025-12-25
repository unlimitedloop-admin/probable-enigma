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
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/bg/BGTileMapProvider.h"
#include "apps/resources/bg/AddressScraper.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/ITileMapProvider.h"
#include "apps/systems/physics/LadderService.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/physics/TileQueryService.h"
#include "apps/systems/scrolling/atomic/MapRenderer2D.h"
#include "apps/systems/scrolling/atomic/ScraperScrollRuleProvider.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "apps/world/stage/RoomGraphAdapter.h"
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
            using ILadderService            = systems::physics::ILadderService;
            using ITerrainProbe             = systems::physics::ITerrainProbe;
            using ITileMapProvider          = systems::physics::ITileMapProvider;
            using LadderService             = systems::physics::LadderService;
            using PageGridIndex             = systems::physics::PageGridIndex;
            using TileQueryService          = systems::physics::TileQueryService;

            using PlayerEntity              = world::entity::avatar::PlayerEntity;
            using RoomGraphAdapter          = world::stage::RoomGraphAdapter;
            using Vec2                      = foundation::math::Vec2;

        public:
            explicit MainPhase(DemoStage1& owner) : owner(owner) {}
            ~MainPhase() override = default;

            void Initialize() override;
            void Update() override;
            void RenderWorld() override;
            void RenderOverlay() override;
            DemoStage1PhaseId Id() const noexcept override;

        private:
            const std::wstring kClassName{ L"DemoStage1::MainPhase" };
            const int kTilePx{ config::SystemConfig::kTileSize };

            DemoStage1& owner;

            std::unique_ptr<ITileMapProvider> _mapProvider;         // Tile map provider
            std::unique_ptr<ITerrainProbe>    _terrainProbe;        // Terrain probe
            std::unique_ptr<ILadderService>   _ladderService;       // Laddering action service
            std::unique_ptr<PageGridIndex>    _pageGrid;            // Page grid index

            std::unique_ptr<MapRenderer2D>             _renderer;   // Map renderer
            std::unique_ptr<ScraperScrollRuleProvider> _rules;      // Scroll rule provider
            std::unique_ptr<ScrollController>          _scroll;     // Scroll controller

            std::unique_ptr<PlayerEntity>     _player;              // Player entity
            std::unique_ptr<RoomGraphAdapter> _graph;               // Room graph adapter

            int _page_index_debug{ 0 };
            double _player_pos_x_debug{ 0 };
            double _player_pos_y_debug{ 0 };

            const Vec2 _initialize_pos{ 128.0, 10.0 };              // HACK: Received from an external class.
            Vec2 _player_prev_pos{};                                // Previous player position, scrolling-player sync use
        };
    }
}