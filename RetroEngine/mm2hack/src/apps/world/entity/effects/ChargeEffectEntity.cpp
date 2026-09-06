#include "pch.h"

#include "ChargeEffectEntity.h"

#include "apps/runtime/GameContext.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnChargeEffectCommand.h"

namespace mm2hack::apps::world::entity::effects
{
    ChargeEffectEntity::ChargeEffectEntity(const common::SpawnChargeEffectCommand& command)
        : _id(command.spriteId), _base_texture(command.baseTexture)
    {
        pos = command.spawnPos;
    }

    ChargeEffectEntity::ChargeEffectEntity(
        const TimedEffectEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        RestoreState(state);
    }

    bool ChargeEffectEntity::SaveState(core::save::StateWriter& writer) const
    {
        return CaptureState().Save(writer, kTotalTicks);
    }

    TimedEffectEntityState ChargeEffectEntity::CaptureState() const noexcept
    {
        return TimedEffectEntityState{
            CaptureKinematicState(),
            _base_texture,
            _elapsed_ticks
        };
    }

    bool ChargeEffectEntity::RestoreState(const TimedEffectEntityState& state) noexcept
    {
        if (!state.IsValid(kTotalTicks) || !RestoreKinematicState(state.kinematic))
        {
            return false;
        }
        _base_texture = state.base_texture;
        _elapsed_ticks = state.elapsed_ticks;
        return true;
    }

    systems::view::Layer ChargeEffectEntity::DrawLayer() const noexcept
    {
        return systems::view::Layer::Effects;
    }

    void ChargeEffectEntity::Update(const systems::view::ViewState* view, double dt)
    {
        (void)view;
        (void)dt;

        if (!IsAlive()) return;

        pos.y -= 2.0;
        ++_elapsed_ticks;
        if (_elapsed_ticks >= kTotalTicks)
        {
            Kill();
        }
    }

    void ChargeEffectEntity::Render(systems::view::RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr) return;

        constexpr double kHalfTile = 8.0;
        int frame = 0;
        int remaining_ticks = _elapsed_ticks;
        while (frame < static_cast<int>(kFrameDurations.size()) - 1 &&
               remaining_ticks >= kFrameDurations[frame])
        {
            remaining_ticks -= kFrameDurations[frame];
            ++frame;
        }
        const int screen_x = static_cast<int>(pos.x - ctx.view->viewWorldX - kHalfTile);
        const int screen_y = static_cast<int>(pos.y - ctx.view->viewWorldY - kHalfTile);

        auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
        resource.GetSpriteManager().UseById(
            _id,
            _base_texture + frame,
            screen_x,
            screen_y);
    }
}
