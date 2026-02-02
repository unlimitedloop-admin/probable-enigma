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
#include "apps/scenes/IStageAssetProvider.h"
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
        const scenes::IStageAssetProvider* asset_provider{ nullptr };
    };

    // Builder for Action Stage Runtime Context
    class ActionStageRuntimeBuilder final
    {
        using ResourceManager = resources::ResourceManager;

    public:
        ActionStageRuntimeBuilder() = default;
        ~ActionStageRuntimeBuilder() = default;

        ActionStageRuntimeBuilder(const ActionStageRuntimeBuilder&) = delete;
        ActionStageRuntimeBuilder& operator=(const ActionStageRuntimeBuilder&) = delete;

        // Build the StageRuntimeContext based on the provided StageDefinition and build config
        [[nodiscard]] std::unique_ptr<StageRuntimeContext> Build(
            ResourceManager& resource,
            core::assembly::StateProvider& input,
            const StageDefinition& def,
            const ActionStageBuildConfig& config,
            const std::wstring& area_key
        ) const;

    private:
        const std::wstring kClassName{ L"ActionStageRuntimeBuilder" };

        void buildCore_(
            StageRuntimeContext& ctx,
            ResourceManager& resource,
            const StageDefinition& def,
            const ActionStageBuildConfig& config
        ) const;    // Build core systems (map, scrolling, physics)
        void buildEntities_(
            StageRuntimeContext& ctx,
            const StageDefinition& def,
            const ActionStageBuildConfig& config
        ) const;    // Build entities (player, enemies, items, etc.)
    };
}