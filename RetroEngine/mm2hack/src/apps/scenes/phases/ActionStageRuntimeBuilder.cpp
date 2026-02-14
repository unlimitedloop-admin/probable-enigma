#include "pch.h"

#include "ActionStageRuntimeBuilder.h"

#include "apps/rendering/bg/BGTileMapProvider.h"
#include "apps/resources/ResourceManager.h"
#include "apps/systems/physics/LadderService.h"
#include "apps/systems/physics/PageGridIndex.h"
#include "apps/systems/physics/TileQueryService.h"
#include "apps/systems/scrolling/atomic/Camera.h"
#include "apps/systems/scrolling/atomic/ScrollController.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "apps/world/entity/EntityManager.h"
#include "apps/world/stage/RoomGraphAdapter.h"
#include "config/SystemConfig.h"
#include "core/assembly/StateProvider.h"
#include "StageDefinition.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::scenes::phases
{
    std::unique_ptr<StageRuntimeContext> ActionStageRuntimeBuilder::Build(
        ResourceManager& resource,
        core::assembly::StateProvider& input,
        const StageDefinition& def,
        const ActionStageBuildConfig& config,
        const std::wstring& area_key
    ) const
    {
        auto ctx = std::make_unique<StageRuntimeContext>();
        ctx->input = &input;
        ctx->area_key = area_key;

        buildCore_(*ctx, resource, def, config);
        buildEntities_(*ctx, def, config);

        return ctx;
    }

    void ActionStageRuntimeBuilder::buildCore_(
        StageRuntimeContext& ctx,
        ResourceManager& resource,
        const StageDefinition& def,
        const ActionStageBuildConfig& config
    ) const
    {
        using resources::bg::AddressScraper;
        using resources::bg::MapPageCache;
        using systems::physics::PageGridIndex;
        using systems::scrolling::atomic::MapRenderer2D;
        using systems::scrolling::atomic::ScraperScrollRuleProvider;
        using systems::scrolling::atomic::ScrollController;

        // 1) Create scraper / page cache from BGRoomBank binary
        //    NOTE: We assume the Scene already loaded BGRoomBank with def.map_binary_path.
        //          We reuse the bank's resolved file path for ExtractMapBinary().
        auto* bg_mgr = &resource.GetBGTileManager();
        auto& filepath = resource.GetBGRoomBank().FilePath();
        ctx.scraper = std::make_shared<AddressScraper>(resource.GetBGTileManager().ExtractMapBinary(filepath));
        ctx.page_source = std::make_shared<MapPageCache>(ctx.scraper);

        // 2) Build graph + page grid
        constexpr double kPageW = static_cast<double>(config::SystemConfig::kScreenWidth);
        constexpr double kPageH = static_cast<double>(config::SystemConfig::kScreenHeight);

        ctx.page_grid = std::make_unique<PageGridIndex>(kPageW, kPageH);
        ctx.graph = std::make_unique<world::stage::RoomGraphAdapter>(*ctx.scraper);

        // start index policy:
        // - If you want stable embedding relative to page 0, keep 0.
        // - If you want per-area embedding (warp to disconnected component), use def.start_page_index.
        ctx.page_grid->Build(*ctx.graph, /*start*/ 0);

        // 3) Renderer / rules / scroll
        ctx.rules = std::make_unique<ScraperScrollRuleProvider>(ctx.page_source);
        ctx.renderer = std::make_unique<MapRenderer2D>(
            resource,
            config.map_name,
            def.map_binary_path,
            config.tile_px
        );

        ScrollController::Params p{};   // Use default params
        ctx.scroll = std::make_unique<ScrollController>(*ctx.rules, *ctx.renderer, p);
        ctx.scroll->SetPageIndex(static_cast<std::size_t>(def.start_page_index));
        ctx.scroll->ObjectPos() = {
            systems::scrolling::atomic::Camera::kCenterX,
            systems::scrolling::atomic::Camera::kCenterY
        };

        // 4) Tile map provider / terrain probe / ladder
        ctx.map_provider = std::make_unique<rendering::bg::BGTileMapProvider>(bg_mgr, ctx.page_source);

        ctx.terrain_probe = std::make_unique<systems::physics::TileQueryService>(
            *ctx.map_provider,
            *ctx.graph,
            *ctx.page_grid,
            config.tile_px
        );

        ctx.ladder_service = std::make_unique<systems::physics::LadderService>(*ctx.terrain_probe);
    }

    void ActionStageRuntimeBuilder::buildEntities_(
        StageRuntimeContext& ctx,
        const StageDefinition& def,
        const ActionStageBuildConfig& config
    ) const
    {
        using world::entity::EntityManager;
        using world::entity::avatar::PlayerEntity;

        ctx.asset_provider = config.asset_provider;
        ctx.entity_mgr = std::make_unique<EntityManager>();

        const auto player_sprite = ctx.asset_provider->PlayerSprite();
        const auto player_attack_sprite = ctx.asset_provider->PlayerAttackSprite();
        auto* player = &ctx.entity_mgr->Spawn<PlayerEntity>(player_sprite, player_attack_sprite);

        player->SetTerrainProbe(ctx.terrain_probe.get());
        player->SetLadderService(ctx.ladder_service.get());
        player->SetScrollContext(ctx.rules.get(), ctx.scroll->PageIndex());
        // Convert local start pos -> world pos on start page
        // NOTE: function name in your code is ToWorldPosOnPage(...)
        if (ctx.page_grid)
        {
            const int page = static_cast<int>(ctx.scroll->PageIndex());
            if (const auto world_pos = ctx.page_grid->ToWorldPosOnPage(page, def.start_local_pos))
            {
                player->pos = *world_pos;
            }
            else
            {
                player->pos = def.start_local_pos; // fallback
            }
        }
        else
        {
            player->pos = def.start_local_pos; // fallback
        }

        player->texture = 0;
    }
}