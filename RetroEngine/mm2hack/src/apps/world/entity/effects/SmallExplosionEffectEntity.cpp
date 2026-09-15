#include "pch.h"

#include "SmallExplosionEffectEntity.h"

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnSmallExplosionEffectCommand.h"
#include "apps/world/entity/EntityBase.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::effects
{
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    SmallExplosionEffectEntity::SmallExplosionEffectEntity(const common::SpawnSmallExplosionEffectCommand& command)
        : _id(command.spriteId), _base_texture(command.baseTexture)
    {
        pos = command.spawnPos;
    }

    SmallExplosionEffectEntity::SmallExplosionEffectEntity(
        const TimedEffectEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        RestoreState(state);
    }

    bool SmallExplosionEffectEntity::SaveState(core::save::StateWriter& writer) const
    {
        return CaptureState().Save(writer, kTotalTicks);
    }

    TimedEffectEntityState SmallExplosionEffectEntity::CaptureState() const noexcept
    {
        return TimedEffectEntityState{
            CaptureKinematicState(),
            _base_texture,
            _elapsed_ticks
        };
    }

    bool SmallExplosionEffectEntity::RestoreState(const TimedEffectEntityState& state) noexcept
    {
        if (!state.IsValid(kTotalTicks) || !RestoreKinematicState(state.kinematic))
        {
            return false;
        }
        _base_texture = state.base_texture;
        _elapsed_ticks = state.elapsed_ticks;
        return true;
    }

    Layer SmallExplosionEffectEntity::DrawLayer() const noexcept
    {
        return Layer::Effects;
    }

    void SmallExplosionEffectEntity::Update(const ViewState* view, double dt)
    {
        (void)view;
        (void)dt;

        if (!IsAlive())
        {
            return;
        }

        ++_elapsed_ticks;
        if (_elapsed_ticks >= kTotalTicks)
        {
            Kill();
        }
    }

    void SmallExplosionEffectEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        // Walk the stage table the same way ChargeEffectEntity does: subtract each
        // stage's duration from the elapsed ticks until landing on the active one.
        int stage_index = 0;
        int remaining_ticks = _elapsed_ticks;
        while (stage_index < static_cast<int>(kStages.size()) - 1 &&
               remaining_ticks >= kStages[stage_index].duration)
        {
            remaining_ticks -= kStages[stage_index].duration;
            ++stage_index;
        }
        const Stage& stage = kStages[stage_index];

        const auto& view = *ctx.view;
        auto& sprites = runtime::GameContext::GetInstance().GetResourceManager().GetSpriteManager();
        const int tile = _base_texture + stage.tile;

        if (stage.composite)
        {
            // 2x2 block (32x32), centered on pos.
            constexpr double kHalf = 16.0;
            const int x = static_cast<int>(pos.x - view.viewWorldX - kHalf);
            const int y = static_cast<int>(pos.y - view.viewWorldY - kHalf);
            sprites.UseById(_id, tile,     x,      y);
            sprites.UseById(_id, tile + 1, x + 16, y);
            sprites.UseById(_id, tile + 8, x,      y + 16);
            sprites.UseById(_id, tile + 9, x + 16, y + 16);
        }
        else
        {
            // Single 16x16 tile, centered on pos.
            constexpr double kHalf = 8.0;
            const int x = static_cast<int>(pos.x - view.viewWorldX - kHalf);
            const int y = static_cast<int>(pos.y - view.viewWorldY - kHalf);
            sprites.UseById(_id, tile, x, y);
        }
    }
}
