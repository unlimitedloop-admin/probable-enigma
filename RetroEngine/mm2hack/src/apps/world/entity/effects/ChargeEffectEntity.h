//==============================================================================
//
//  Project: mm2hack
//  ChargeEffectEntity.h
//
//  One rising particle displayed while the player charges the Rock Buster.
//
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <array>
#include <string>
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnChargeEffectCommand.h"

namespace mm2hack::apps::world::entity::effects
{
    class ChargeEffectEntity final : public EntityBase
    {
    public:
        explicit ChargeEffectEntity(const common::SpawnChargeEffectCommand& command);

        systems::view::Layer DrawLayer() const noexcept override;
        void Update(const systems::view::ViewState* view, double dt) override;
        void Render(systems::view::RenderContext& ctx) override;

    private:
        const std::wstring kClassName{ L"ChargeEffectEntity" };

        static constexpr std::array<int, 6> kFrameDurations{ 3, 5, 5, 3, 2, 1 };

        rendering::sprite::SpriteManager::Id _id{};
        int _base_texture{ 0 };
        int _elapsed_ticks{ 0 };
    };
}
