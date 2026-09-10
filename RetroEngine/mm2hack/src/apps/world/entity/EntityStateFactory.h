//==============================================================================
//
//  Project: mm2hack
//  EntityStateFactory.h
//
//  Reconstructs entities from validated logical state records.
//
//==============================================================================
#pragma once

#include <memory>

#include "apps/scenes/IStageAssetProvider.h"
#include "EntityManager.h"
#include "IEntity.h"

namespace mm2hack::apps::world::entity
{
    class EntityStateFactory final
    {
    public:
        explicit EntityStateFactory(const scenes::IStageAssetProvider& assets) noexcept
            : _assets(assets)
        {
        }

        [[nodiscard]] static bool ValidateRecord(const EntityStateRecord& record);
        [[nodiscard]] std::unique_ptr<IEntity> Create(const EntityStateRecord& record) const;

    private:
        const scenes::IStageAssetProvider& _assets;
    };
}
