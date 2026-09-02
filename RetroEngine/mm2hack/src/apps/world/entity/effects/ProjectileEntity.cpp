#include "pch.h"

#include "ProjectileEntity.h"

#include <algorithm>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"

namespace mm2hack::apps::world::entity::effects
{
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    ProjectileEntity::ProjectileEntity(const common::SpawnProjectileCommand& cmd)
    {
        pos = cmd.spawnPos;
        vel = cmd.velocity;

        _id = cmd.spriteId;
        _draw_layer = cmd.drawLayer;
        _base_texture = cmd.baseTexture;
        _visual = cmd.visual;
        _anim_frames = std::max<std::int32_t>(1, cmd.animFrames);
        _anim_fps = std::max(0.0, cmd.animFps);

        _life_sec = std::max(0.0, cmd.lifeSec);
        _age_sec = 0.0;

        _half = foundation::math::Vec2{ 16.0, 16.0 };   // Assuming an average size; adjust as needed.
    }

    Layer ProjectileEntity::DrawLayer() const noexcept
    {
        return _draw_layer;
    }

    void ProjectileEntity::Update(const ViewState* view, double dt)
    {
        if (!IsAlive())
        {
            return;
        }

        pos += vel * dt;
        _age_sec += dt;
        ++_elapsed_ticks;

        constexpr double margin = 32.0;

        const double left = view->viewWorldX - margin;
        const double right = view->viewWorldX + view->viewW + margin;
        const double top = view->viewWorldY - margin;
        const double bottom = view->viewWorldY + view->viewH + margin;

        if (pos.x < left || pos.x > right || pos.y < top || pos.y > bottom)
        {
            Kill();
        }
    }

    void ProjectileEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        const auto& view = *ctx.view;
        const double worldX = pos.x;
        const double worldY = pos.y;

        const double screenX = worldX - view.viewWorldX - _half.x;
        const double screenY = worldY - view.viewWorldY - _half.y;

        auto& res = runtime::GameContext::GetInstance().GetResourceManager();
        auto& sprites = res.GetSpriteManager();
        const int x = static_cast<int>(screenX);
        const int y = static_cast<int>(screenY);

        if (_visual == common::ProjectileVisual::ChargeLevel1)
        {
            const int frame = static_cast<int>((_elapsed_ticks / 3u) % 2u);
            const int first_tile = _base_texture + 8 + frame * 2;
            sprites.UseById(_id, first_tile, x, y);
            sprites.UseById(_id, first_tile + 1, x + 16, y);
            return;
        }

        if (_visual == common::ProjectileVisual::ChargeLevel2)
        {
            const int frame = static_cast<int>((_elapsed_ticks / 3u) % 4u);
            const int top_left = _base_texture + 16 + frame * 2;
            constexpr int kCenterOffsetY = -8;
            const int centered_y = y + kCenterOffsetY;
            sprites.UseById(_id, top_left, x, centered_y);
            sprites.UseById(_id, top_left + 1, x + 16, centered_y);
            sprites.UseById(_id, top_left + 8, x, centered_y + 16);
            sprites.UseById(_id, top_left + 9, x + 16, centered_y + 16);
            return;
        }

        int texture = _base_texture;
        if (_anim_frames > 1 && _anim_fps > 0.0)
        {
            const double frame_d = _age_sec * _anim_fps;
            const std::int32_t frame = static_cast<std::int32_t>(frame_d) % _anim_frames;
            texture += static_cast<int>(frame);
        }
        sprites.UseById(_id, texture, x, y);
    }
}
