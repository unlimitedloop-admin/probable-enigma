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
        constexpr int kTotalTicks = 3 + 5 + 5 + 3 + 2 + 1;
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
