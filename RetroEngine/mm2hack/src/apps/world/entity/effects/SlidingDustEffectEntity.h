//==============================================================================
//
//  Project: mm2hack
//  SlidingDustEffectEntity.h
//
//  One-shot dust animation displayed when the player begins sliding.
//
//==============================================================================
#pragma once

#include <array>
#include "apps/world/entity/EntityBase.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/world/entity/common/SpawnSlidingDustEffectCommand.h"

namespace mm2hack::apps::world::entity::effects
{
    class SlidingDustEffectEntity final : public EntityBase
    {
    public:
        explicit SlidingDustEffectEntity(const common::SpawnSlidingDustEffectCommand& command);

        systems::view::Layer DrawLayer() const noexcept override;
        void Update(const systems::view::ViewState* view, double dt) override;
        void Render(systems::view::RenderContext& ctx) override;

    private:
        static constexpr std::array<int, 4> kFrameDurations{ 8, 9, 9, 9 };

        rendering::sprite::SpriteManager::Id _id{};
        int _base_texture{ 0 };
        int _elapsed_ticks{ 0 };
    };
}
