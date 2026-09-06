#include "pch.h"

#include "SplashEffectEntity.h"

#include "apps/runtime/GameContext.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnSplashEffectCommand.h"

namespace mm2hack::apps::world::entity::effects
{
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    SplashEffectEntity::SplashEffectEntity(const common::SpawnSplashEffectCommand& command)
        : _id(command.spriteId), _base_texture(command.baseTexture)
    {
        pos = command.spawnPos;
    }

    SplashEffectEntity::SplashEffectEntity(
        const TimedEffectEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        RestoreState(state);
    }

    bool SplashEffectEntity::SaveState(core::save::StateWriter& writer) const
    {
        return CaptureState().Save(writer, kTotalTicks);
    }

    TimedEffectEntityState SplashEffectEntity::CaptureState() const noexcept
    {
        return TimedEffectEntityState{
            CaptureKinematicState(),
            _base_texture,
            _elapsed_ticks
        };
    }

    bool SplashEffectEntity::RestoreState(const TimedEffectEntityState& state) noexcept
    {
        if (!state.IsValid(kTotalTicks) || !RestoreKinematicState(state.kinematic))
        {
            return false;
        }
        _base_texture = state.base_texture;
        _elapsed_ticks = state.elapsed_ticks;
        return true;
    }

    Layer SplashEffectEntity::DrawLayer() const noexcept
    {
        return Layer::Effects;
    }

    void SplashEffectEntity::Update(const ViewState* view, double dt)
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

    void SplashEffectEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        constexpr double kHalfSize = 16.0;
        const int frame = _elapsed_ticks / kTicksPerFrame;
        const int texture = _base_texture + frame;
        const int screen_x = static_cast<int>(pos.x - ctx.view->viewWorldX - kHalfSize);
        const int screen_y = static_cast<int>(pos.y - ctx.view->viewWorldY - kHalfSize);

        auto& resource = runtime::GameContext::GetInstance().GetResourceManager();
        resource.GetSpriteManager().UseById(_id, texture, screen_x, screen_y);
    }
}
