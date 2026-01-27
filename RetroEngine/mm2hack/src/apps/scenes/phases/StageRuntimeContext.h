//==============================================================================
// 
//  Project: mm2hack
//  StageRuntimeContext.h
// 
//  Properties container for stage runtime systems and entities.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>
#include "apps/resources/bg/AddressScraper.h"
#include "apps/resources/bg/MapPageCache.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/ITileMapProvider.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/scrolling/atomic/MapRenderer2D.h"
#include "apps/systems/scrolling/atomic/ScraperScrollRuleProvider.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/world/entity/EntityManager.h"
#include "apps/world/stage/RoomGraphAdapter.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::scenes::phases
{
    // Container struct for stage runtime context, holding various systems and entities.
    struct StageRuntimeContext final
    {
        core::assembly::StateProvider* input{ nullptr };                                // Reference to the raw input provider

        std::wstring area_key{};                                                        // Current area key

        std::shared_ptr<resources::bg::AddressScraper> scraper{};                       // Shared AddressScraper for map data
        std::shared_ptr<resources::bg::MapPageCache> page_source{};                     // Shared MapPageCache for map data
        std::unique_ptr<systems::physics::PageGridIndex> page_grid{};                   // Unique PageGridIndex for physics
        std::unique_ptr<world::stage::RoomGraphAdapter> graph{};                        // Unique RoomGraphAdapter for stage graph

        std::unique_ptr<systems::scrolling::atomic::MapRenderer2D> renderer{};          // Unique MapRenderer2D for scrolling
        std::unique_ptr<systems::scrolling::atomic::ScraperScrollRuleProvider> rules{}; // Unique ScraperScrollRuleProvider for scrolling
        std::unique_ptr<systems::scrolling::atomic::ScrollController> scroll{};         // Unique ScrollController for scrolling

        std::unique_ptr<systems::physics::ITileMapProvider> map_provider{};             // Unique ITileMapProvider for physics
        std::unique_ptr<systems::physics::ITerrainProbe> terrain_probe{};               // Unique ITerrainProbe for physics
        std::unique_ptr<systems::physics::ILadderService> ladder_service{};             // Unique ILadderService for physics

        std::unique_ptr<world::entity::EntityManager> entity_mgr{};                     // Unique EntityManager for entities
    };
}