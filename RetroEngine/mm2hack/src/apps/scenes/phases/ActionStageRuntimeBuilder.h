//==============================================================================
// 
//  Project: mm2hack
//  ActionStageRuntimeBuilder.h
// 
//  Defines build config and builder for Action Stage Runtime Context.
// 
//==============================================================================
#pragma once

#include <memory>
#include <string>

#include "apps/resources/ResourceManager.h"
#include "core/assembly/StateProvider.h"
#include "StageDefinition.h"
#include "StageRuntimeContext.h"

namespace mm2hack::apps::scenes::phases
{
    // Extra build-time info that is not part of StageDefinition
    // Scene supplies these (e.g., map name, tile size, player sprite id)
    struct ActionStageBuildConfig final
    {
        std::wstring map_name{};
        int tile_px{};
        int player_sprite_id{}; // Adjust type to your SpriteManager::Id if needed.
    };

    // Builder for Action Stage Runtime Context
    class ActionStageRuntimeBuilder final
    {
    public:
        ActionStageRuntimeBuilder() = default;
        ~ActionStageRuntimeBuilder() = default;

        ActionStageRuntimeBuilder(const ActionStageRuntimeBuilder&) = delete;
        ActionStageRuntimeBuilder& operator=(const ActionStageRuntimeBuilder&) = delete;

        // Build the StageRuntimeContext based on the provided StageDefinition and build config
        [[nodiscard]] std::unique_ptr<StageRuntimeContext> Build(
            resources::ResourceManager& resource,
            core::assembly::StateProvider& input,
            const StageDefinition& def,
            const ActionStageBuildConfig& config,
            const std::wstring& area_key) const;

    private:
        // Build core systems (map, scrolling, physics)
        void buildCore_(StageRuntimeContext& ctx, resources::ResourceManager& resource, const StageDefinition& def, const ActionStageBuildConfig& config) const;
        // Build entities (player, enemies, items, etc.)
        void buildEntities_(StageRuntimeContext& ctx, const StageDefinition& def, const ActionStageBuildConfig& config) const;
    };
}