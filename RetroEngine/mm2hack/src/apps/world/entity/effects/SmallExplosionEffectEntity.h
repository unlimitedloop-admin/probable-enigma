//==============================================================================
//
//  Project: mm2hack
//  SmallExplosionEffectEntity.h
//
//  One-shot small destruction VFX, played once when a breakable object (and
//  later, a weak enemy) is destroyed. Source sheet: EFFECT11_N0_ALL_PATTERN
//  (128x64px, 16x16 tiles, 8 columns x 4 rows).
//
//==============================================================================
#pragma once

#include "apps/world/entity/EntityBase.h"

#include <array>
#include <cstdint>

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnSmallExplosionEffectCommand.h"
#include "apps/world/entity/IEntity.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::effects
{
    // Five explosion stages (the first two are 2x2 composites, the rest single
    // tiles), tile indices relative to the sprite-set's base texture:
    //   stage0: {0,1 / 8,9}   (32x32) x3 ticks
    //   stage1: {2,3 / 10,11} (32x32) x2 ticks
    //   stage2: {4}           (16x16) x2 ticks
    //   stage3: {5}           (16x16) x2 ticks
    //   stage4: {6}           (16x16) x1 tick
    // Tile 7 is a deliberately blank tile in the sheet (not drawn) marking the
    // end of the sequence; the entity simply dies once stage4's tick is spent.
    class SmallExplosionEffectEntity final : public EntityBase
    {
    public:
        static constexpr std::uint16_t kStateVersion{ 1 };
        static constexpr std::int32_t kTotalTicks{ 10 }; // 3 + 2 + 2 + 2 + 1

        explicit SmallExplosionEffectEntity(const common::SpawnSmallExplosionEffectCommand& command);
        SmallExplosionEffectEntity(
            const TimedEffectEntityState& state,
            rendering::sprite::SpriteManager::Id sprite_id);

        // Get drawing layer (IRenderable)
        systems::view::Layer DrawLayer() const noexcept override;
        // Advance the one-shot animation (IUpdatable)
        void Update(const systems::view::ViewState* view, double dt) override;
        // Draw the current animation stage (IRenderable)
        void Render(systems::view::RenderContext& ctx) override;
        [[nodiscard]] EntityTypeId StateTypeId() const noexcept override { return EntityTypeId::SmallExplosionEffect; }
        [[nodiscard]] std::uint16_t StateComponentVersion() const noexcept override { return kStateVersion; }
        bool SaveState(core::save::StateWriter& writer) const override;
        [[nodiscard]] TimedEffectEntityState CaptureState() const noexcept;
        bool RestoreState(const TimedEffectEntityState& state) noexcept;

    private:
        // One animation stage: `tile` (relative to _base_texture) drawn either as a
        // single 16x16 tile, or -- when `composite` -- as a 2x2 (32x32) block
        // {tile, tile+1, tile+8, tile+9}, matching the sheet's 8-tiles-per-row stride.
        struct Stage final
        {
            int tile{};
            int duration{};
            bool composite{};
        };

        static constexpr std::array<Stage, 5> kStages{ {
            Stage{ 0, 3, true },
            Stage{ 2, 2, true },
            Stage{ 4, 2, false },
            Stage{ 5, 2, false },
            Stage{ 6, 1, false },
        } };

        rendering::sprite::SpriteManager::Id _id{};   // Explosion sprite ID
        int _base_texture{ 0 };                        // First tile index for this sprite-set
        int _elapsed_ticks{ 0 };                       // Number of completed update ticks
    };
}
