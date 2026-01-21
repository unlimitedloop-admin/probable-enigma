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
    // TODO: Replace with your planned manager type (EntityManager / EntityWorld).
    //class EntityManager;

    struct StageRuntimeContext final
    {
        core::assembly::StateProvider* input{ nullptr };    // Reference to the raw input provider

        std::wstring area_key{};

        // Keep these alive (rules/mapProvider/graph may depend on them).
        std::shared_ptr<resources::bg::AddressScraper> scraper{};
        std::shared_ptr<resources::bg::MapPageCache> page_source{};

        std::unique_ptr<systems::physics::PageGridIndex> page_grid{};
        std::unique_ptr<world::stage::RoomGraphAdapter> graph{};

        std::unique_ptr<systems::scrolling::atomic::MapRenderer2D> renderer{};
        std::unique_ptr<systems::scrolling::atomic::ScraperScrollRuleProvider> rules{};
        std::unique_ptr<systems::scrolling::atomic::ScrollController> scroll{};

        std::unique_ptr<systems::physics::ITileMapProvider> map_provider{};
        std::unique_ptr<systems::physics::ITerrainProbe> terrain_probe{};
        std::unique_ptr<systems::physics::ILadderService> ladder_service{};
        
        std::unique_ptr<world::entity::EntityManager> entity_mgr{};
    };
}