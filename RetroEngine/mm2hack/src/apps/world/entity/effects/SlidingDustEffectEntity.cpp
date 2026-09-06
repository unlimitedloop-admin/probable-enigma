#include "pch.h"

#include "SlidingDustEffectEntity.h"

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnSlidingDustEffectCommand.h"
#include "apps/world/entity/EntityBase.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::effects
{
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    SlidingDustEffectEntity::SlidingDustEffectEntity(
        const common::SpawnSlidingDustEffectCommand& command)
        : _id(command.spriteId), _base_texture(command.baseTexture)
    {
        pos = command.spawnPos;
    }

    SlidingDustEffectEntity::SlidingDustEffectEntity(
        const TimedEffectEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        RestoreState(state);
    }

    bool SlidingDustEffectEntity::SaveState(core::save::StateWriter& writer) const
    {
        return CaptureState().Save(writer, kTotalTicks);
    }

    TimedEffectEntityState SlidingDustEffectEntity::CaptureState() const noexcept
    {
        return TimedEffectEntityState{
            CaptureKinematicState(),
            _base_texture,
            _elapsed_ticks
        };
    }

    bool SlidingDustEffectEntity::RestoreState(const TimedEffectEntityState& state) noexcept
    {
        if (!state.IsValid(kTotalTicks) || !RestoreKinematicState(state.kinematic))
        {
            return false;
        }
        _base_texture = state.base_texture;
        _elapsed_ticks = state.elapsed_ticks;
        return true;
    }

    Layer SlidingDustEffectEntity::DrawLayer() const noexcept
    {
        return Layer::Effects;
    }

    void SlidingDustEffectEntity::Update(const ViewState* view, double dt)
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

    void SlidingDustEffectEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        int frame = 0;
        int remaining_ticks = _elapsed_ticks;
        while (frame < static_cast<int>(kFrameDurations.size()) - 1 &&
               remaining_ticks >= kFrameDurations[frame])
        {
            remaining_ticks -= kFrameDurations[frame];
            ++frame;
        }

        constexpr double kHalfSize = 16.0;
        const int screen_x = static_cast<int>(pos.x - ctx.view->viewWorldX - kHalfSize);
        const int screen_y = static_cast<int>(pos.y - ctx.view->viewWorldY - kHalfSize);

        auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
        resource.GetSpriteManager().UseById(
            _id,
            _base_texture + frame,
            screen_x,
            screen_y);
    }
}
