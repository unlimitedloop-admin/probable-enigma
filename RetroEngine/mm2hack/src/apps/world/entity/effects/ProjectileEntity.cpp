#include "pch.h"

#include "ProjectileEntity.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
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

    bool ProjectileEntityState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() && kinematic.Save(writer) &&
            writer.WriteU8(static_cast<std::uint8_t>(draw_layer)) &&
            writer.WriteI32(base_texture) &&
            writer.WriteU8(static_cast<std::uint8_t>(visual)) &&
            writer.WriteI32(animation_frames) &&
            writer.WriteF64(animation_fps) &&
            writer.WriteF64(lifetime_seconds) &&
            writer.WriteF64(age_seconds) &&
            writer.WriteU32(elapsed_ticks);
    }

    bool ProjectileEntityState::Load(core::save::StateReader& reader)
    {
        ProjectileEntityState loaded{};
        std::uint8_t encoded_layer{};
        std::uint8_t encoded_visual{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadU8(encoded_layer) ||
            !reader.ReadI32(loaded.base_texture) ||
            !reader.ReadU8(encoded_visual) ||
            !reader.ReadI32(loaded.animation_frames) ||
            !reader.ReadF64(loaded.animation_fps) ||
            !reader.ReadF64(loaded.lifetime_seconds) ||
            !reader.ReadF64(loaded.age_seconds) ||
            !reader.ReadU32(loaded.elapsed_ticks))
        {
            return false;
        }
        loaded.draw_layer = static_cast<systems::view::Layer>(encoded_layer);
        loaded.visual = static_cast<common::ProjectileVisual>(encoded_visual);
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool ProjectileEntityState::IsValid() const noexcept
    {
        constexpr double kMaximumDurationSeconds = 3'600.0;
        return kinematic.IsValid() &&
            draw_layer >= systems::view::Layer::Background &&
            draw_layer <= systems::view::Layer::Overlay &&
            base_texture >= 0 && base_texture <= 65'535 &&
            visual >= common::ProjectileVisual::Normal &&
            visual <= common::ProjectileVisual::ChargeLevel2 &&
            animation_frames >= 1 && animation_frames <= 1'024 &&
            std::isfinite(animation_fps) && animation_fps >= 0.0 && animation_fps <= 1'000.0 &&
            std::isfinite(lifetime_seconds) && lifetime_seconds >= 0.0 &&
            lifetime_seconds <= kMaximumDurationSeconds &&
            std::isfinite(age_seconds) && age_seconds >= 0.0 &&
            age_seconds <= kMaximumDurationSeconds;
    }

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

    ProjectileEntity::ProjectileEntity(
        const ProjectileEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        _half = foundation::math::Vec2{ 16.0, 16.0 };
        RestoreState(state);
    }

    bool ProjectileEntity::SaveState(core::save::StateWriter& writer) const
    {
        return CaptureState().Save(writer);
    }

    ProjectileEntityState ProjectileEntity::CaptureState() const noexcept
    {
        return ProjectileEntityState{
            .kinematic = CaptureKinematicState(),
            .draw_layer = _draw_layer,
            .base_texture = _base_texture,
            .visual = _visual,
            .animation_frames = _anim_frames,
            .animation_fps = _anim_fps,
            .lifetime_seconds = _life_sec,
            .age_seconds = _age_sec,
            .elapsed_ticks = _elapsed_ticks,
        };
    }

    bool ProjectileEntity::RestoreState(const ProjectileEntityState& state) noexcept
    {
        if (!state.IsValid() || !RestoreKinematicState(state.kinematic))
        {
            return false;
        }
        _draw_layer = state.draw_layer;
        _base_texture = state.base_texture;
        _visual = state.visual;
        _anim_frames = state.animation_frames;
        _anim_fps = state.animation_fps;
        _life_sec = state.lifetime_seconds;
        _age_sec = state.age_seconds;
        _elapsed_ticks = state.elapsed_ticks;
        return true;
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

        if (_age_sec >= _life_sec)
        {
            Kill();
            return;
        }

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
