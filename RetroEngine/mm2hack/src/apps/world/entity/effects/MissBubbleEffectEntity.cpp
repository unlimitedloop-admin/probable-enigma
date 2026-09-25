#include "pch.h"

#include "MissBubbleEffectEntity.h"

#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnMissBubbleEffectCommand.h"

namespace mm2hack::apps::world::entity::effects
{
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    MissBubbleEffectEntity::MissBubbleEffectEntity(const common::SpawnMissBubbleEffectCommand& command)
        : _id(command.spriteId)
    {
        pos = command.spawnPos;
        vel = command.velocity;
    }

    Layer MissBubbleEffectEntity::DrawLayer() const noexcept
    {
        return Layer::Effects;
    }

    void MissBubbleEffectEntity::Update(const ViewState* view, double dt)
    {
        (void)view;
        (void)dt;

        if (!IsAlive())
        {
            return;
        }

        // Never killed on leaving the screen: a pit miss spawns the whole ring
        // below the view, and the upward-bound bubbles still need to rise into
        // it. The stage restart tears the entity manager down anyway.
        pos += vel;
        _elapsed_ticks = (_elapsed_ticks + 1) % (kTicksPerStage * static_cast<int>(kStages.size()));
    }

    void MissBubbleEffectEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        const Stage& stage = kStages[static_cast<std::size_t>(_elapsed_ticks / kTicksPerStage)];
        if (stage.blank)
        {
            return;
        }

        const auto& view = *ctx.view;
        auto& sprites = runtime::GameContext::GetInstance().GetResourceManager().GetSpriteManager();

        if (stage.composite)
        {
            // 2x2 block (32x32), centered on pos.
            constexpr double kHalf = 16.0;
            const int x = static_cast<int>(pos.x - view.viewWorldX - kHalf);
            const int y = static_cast<int>(pos.y - view.viewWorldY - kHalf);
            sprites.UseById(_id, stage.tile,     x,      y);
            sprites.UseById(_id, stage.tile + 1, x + 16, y);
            sprites.UseById(_id, stage.tile + 8, x,      y + 16);
            sprites.UseById(_id, stage.tile + 9, x + 16, y + 16);
        }
        else
        {
            // Single 16x16 tile, centered on pos.
            constexpr double kHalf = 8.0;
            const int x = static_cast<int>(pos.x - view.viewWorldX - kHalf);
            const int y = static_cast<int>(pos.y - view.viewWorldY - kHalf);
            sprites.UseById(_id, stage.tile, x, y);
        }
    }
}
