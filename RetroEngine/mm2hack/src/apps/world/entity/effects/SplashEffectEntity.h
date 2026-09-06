//==============================================================================
//
//  Project: mm2hack
//  SplashEffectEntity.h
//
//  One-shot splash animation displayed when the player enters water.
//
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <cstdint>
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnSplashEffectCommand.h"
#include "apps/world/entity/IEntity.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::effects
{
    // Four-frame splash animation entity
    class SplashEffectEntity final : public EntityBase
    {
    public:
        static constexpr std::uint16_t kStateVersion{ 1 };
        static constexpr std::int32_t kTotalTicks{ 28 };

        explicit SplashEffectEntity(const common::SpawnSplashEffectCommand& command);
        SplashEffectEntity(
            const TimedEffectEntityState& state,
            rendering::sprite::SpriteManager::Id sprite_id);

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Advance the one-shot animation (IUpdatable)
        void Update(const systems::view::ViewState* view, double dt) override;
        // Draw the current animation frame (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::SplashEffect; }
        [[nodiscard]] std::uint16_t StateComponentVersion() const noexcept override { return kStateVersion; }
        bool SaveState(core::save::StateWriter& writer) const override;
        [[nodiscard]] TimedEffectEntityState CaptureState() const noexcept;
        bool RestoreState(const TimedEffectEntityState& state) noexcept;

    private:
        static constexpr int kTicksPerFrame{ 7 };
        static constexpr int kFrameCount{ 4 };

        rendering::sprite::SpriteManager::Id _id{};              // Splash sprite ID
        int _base_texture{ 0 };                                   // First texture for the captured direction
        int _elapsed_ticks{ 0 };                                  // Number of completed update ticks
    };
}
