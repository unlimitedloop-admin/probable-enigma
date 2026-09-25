//==============================================================================
//
//  Project: mm2hack
//  MissBubbleEffectEntity.h
//
//  One bubble of the player's miss (death) effect: drifts in a straight line
//  at a constant speed while looping its pulse animation, until the stage
//  restarts. Source sheet: EFFECT04_N0_ALL_PATTERN (same layout as EFFECT11,
//  128x64px, 16x16 tiles, 8 columns x 4 rows).
//
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <array>
#include <cstdint>

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnMissBubbleEffectCommand.h"
#include "apps/world/entity/IEntity.h"

namespace mm2hack::apps::world::entity::effects
{
    // Six looping stages, 3 ticks each (18-tick cycle), tile indices relative
    // to the sheet (same composite convention as SmallExplosionEffectEntity):
    //   stage0: {0,1 / 8,9}   (32x32) ring
    //   stage1: {2,3 / 10,11} (32x32) filled
    //   stage2: {4}           (16x16)
    //   stage3: {5}           (16x16)
    //   stage4: {6}           (16x16)
    //   stage5: {7}           blank tile -- drawn as nothing, gives the flicker
    //
    // Deliberately not save-state capable (StateComponentVersion() stays 0):
    // it only ever exists during the miss sequence, and AbstractActionPhase::
    // CanCaptureState() refuses a capture for that whole window anyway.
    class MissBubbleEffectEntity final : public EntityBase
    {
    public:
        explicit MissBubbleEffectEntity(const common::SpawnMissBubbleEffectCommand& command);

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Drift and advance the looping animation (IUpdatable)
        void Update(const systems::view::ViewState* view, double dt) override;
        // Draw the current animation stage (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::MissBubbleEffect; }

    private:
        struct Stage final
        {
            int tile{};
            bool composite{};
            bool blank{};
        };

        static constexpr int kTicksPerStage{ 3 };
        static constexpr std::array<Stage, 6> kStages{ {
            Stage{ 0, true,  false },
            Stage{ 2, true,  false },
            Stage{ 4, false, false },
            Stage{ 5, false, false },
            Stage{ 6, false, false },
            Stage{ 7, false, true  },
        } };

        rendering::sprite::SpriteManager::Id _id{};   // Bubble sprite ID
        int _elapsed_ticks{ 0 };                       // Number of completed update ticks
    };
}
