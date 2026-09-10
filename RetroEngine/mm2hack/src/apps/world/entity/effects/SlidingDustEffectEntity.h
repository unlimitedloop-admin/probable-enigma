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
#include <cstdint>

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnSlidingDustEffectCommand.h"
#include "apps/world/entity/EntityBase.h"
#include "apps/world/entity/IEntity.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::effects
{
    class SlidingDustEffectEntity final : public EntityBase
    {
    public:
        static constexpr std::uint16_t kStateVersion{ 1 };
        static constexpr std::int32_t kTotalTicks{ 35 };

        explicit SlidingDustEffectEntity(const common::SpawnSlidingDustEffectCommand& command);
        SlidingDustEffectEntity(
            const TimedEffectEntityState& state,
            rendering::sprite::SpriteManager::Id sprite_id);

        systems::view::Layer DrawLayer() const noexcept override;
        void Update(const systems::view::ViewState* view, double dt) override;
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::SlidingDustEffect; }
        [[nodiscard]] std::uint16_t StateComponentVersion() const noexcept override { return kStateVersion; }
        bool SaveState(core::save::StateWriter& writer) const override;
        [[nodiscard]] TimedEffectEntityState CaptureState() const noexcept;
        bool RestoreState(const TimedEffectEntityState& state) noexcept;

    private:
        static constexpr std::array<int, 4> kFrameDurations{ 8, 9, 9, 9 };

        rendering::sprite::SpriteManager::Id _id{};
        int _base_texture{ 0 };
        int _elapsed_ticks{ 0 };
    };
}
