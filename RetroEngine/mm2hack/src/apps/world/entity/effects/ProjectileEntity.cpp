#include "pch.h"

#include "ProjectileEntity.h"

#include <cmath>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/IDeflector.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::effects
{
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    namespace
    {
        // Render-only centering half-size, derived from the actual footprint each
        // visual draws (PLAYER_WEAPON_N0_ALL_PATTERN's tiles are 16x16px each) --
        // NOT the hit judgement box (see _hit_half_size/Bounds()). Getting this
        // wrong doesn't affect collision, only where the sprite is drawn relative
        // to pos: too large a half-size here shifts the visible sprite away from
        // its true (collision-accurate) position, making contact look like it
        // happens too early/too close for a shot travelling toward that side.
        foundation::math::Vec2 RenderHalfSizeForVisual(common::ProjectileVisual visual) noexcept
        {
            switch (visual)
            {
            case common::ProjectileVisual::ChargeLevel1:
                return { 16.0, 8.0 };   // two 16x16 tiles side by side (32x16)
            case common::ProjectileVisual::ChargeLevel2:
                return { 16.0, 16.0 };  // 2x2 grid of 16x16 tiles (32x32)
            case common::ProjectileVisual::Normal:
            default:
                return { 8.0, 8.0 };    // single 16x16 tile
            }
        }
    }

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
            writer.WriteU32(elapsed_ticks) &&
            writer.WriteI32(power) &&
            writer.WriteF64(hit_half_size.x) &&
            writer.WriteF64(hit_half_size.y) &&
            writer.WriteU8(static_cast<std::uint8_t>(collision_layer)) &&
            writer.WriteU8(static_cast<std::uint8_t>(weapon)) &&
            writer.WriteBool(terrain_collision_enabled);
    }

    bool ProjectileEntityState::Load(core::save::StateReader& reader)
    {
        ProjectileEntityState loaded{};
        std::uint8_t encoded_layer{};
        std::uint8_t encoded_visual{};
        std::uint8_t encoded_collision_layer{};
        std::uint8_t encoded_weapon{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadU8(encoded_layer) ||
            !reader.ReadI32(loaded.base_texture) ||
            !reader.ReadU8(encoded_visual) ||
            !reader.ReadI32(loaded.animation_frames) ||
            !reader.ReadF64(loaded.animation_fps) ||
            !reader.ReadF64(loaded.lifetime_seconds) ||
            !reader.ReadF64(loaded.age_seconds) ||
            !reader.ReadU32(loaded.elapsed_ticks) ||
            !reader.ReadI32(loaded.power) ||
            !reader.ReadF64(loaded.hit_half_size.x) ||
            !reader.ReadF64(loaded.hit_half_size.y) ||
            !reader.ReadU8(encoded_collision_layer) ||
            !reader.ReadU8(encoded_weapon) ||
            !reader.ReadBool(loaded.terrain_collision_enabled))
        {
            return false;
        }
        loaded.draw_layer = static_cast<systems::view::Layer>(encoded_layer);
        loaded.visual = static_cast<common::ProjectileVisual>(encoded_visual);
        loaded.collision_layer = static_cast<systems::physics::CollisionLayer>(encoded_collision_layer);
        loaded.weapon = static_cast<systems::physics::WeaponId>(encoded_weapon);
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
        constexpr double kMaximumHitHalfSize = 256.0;
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
            age_seconds <= kMaximumDurationSeconds &&
            power >= 0 && power <= 1'000 &&
            std::isfinite(hit_half_size.x) && hit_half_size.x > 0.0 && hit_half_size.x <= kMaximumHitHalfSize &&
            std::isfinite(hit_half_size.y) && hit_half_size.y > 0.0 && hit_half_size.y <= kMaximumHitHalfSize &&
            collision_layer < systems::physics::CollisionLayer::Count &&
            weapon < systems::physics::WeaponId::Count;
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

        _half = RenderHalfSizeForVisual(_visual);

        _power = cmd.power;
        _hit_half_size = cmd.hitHalfSize;
        _collision_layer = cmd.collisionLayer;
        _weapon = cmd.weapon;
        _terrain_collision_enabled = cmd.terrainCollisionEnabled;
    }

    ProjectileEntity::ProjectileEntity(
        const ProjectileEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        RestoreState(state);
        _half = RenderHalfSizeForVisual(_visual);
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
            .power = _power,
            .hit_half_size = _hit_half_size,
            .collision_layer = _collision_layer,
            .weapon = _weapon,
            .terrain_collision_enabled = _terrain_collision_enabled,
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
        _power = state.power;
        _hit_half_size = state.hit_half_size;
        _collision_layer = state.collision_layer;
        _weapon = state.weapon;
        _terrain_collision_enabled = state.terrain_collision_enabled;
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

        // A non-positive lifetime is the existing sentinel for an entity that
        // remains alive until it leaves the active view.
        if (_life_sec > 0.0 && _age_sec >= _life_sec)
        {
            Kill();
            return;
        }

        if (checkTerrainCollision_())
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

    ProjectileEntity::RectF ProjectileEntity::Bounds() const
    {
        // Deliberately independent of _half (the sprite's draw footprint): the
        // attack hit judgement box is its own, generally smaller, concept.
        return { pos.x - _hit_half_size.x, pos.y - _hit_half_size.y,
                 _hit_half_size.x * 2.0, _hit_half_size.y * 2.0 };
    }

    void ProjectileEntity::OnTileCollision(const Vec2& normal, TileAttribute attr)
    {
        // Unused: nothing in this codebase drives entities through ICollider::
        // OnTileCollision yet (PlayerEntity/EnemyEntity resolve terrain directly
        // in their own Update() too). See checkTerrainCollision_() for the actual
        // wall/floor despawn logic.
        (void)normal;
        (void)attr;
    }

    bool ProjectileEntity::checkTerrainCollision_() const
    {
        if (!_terrain_collision_enabled || _terrain == nullptr)
        {
            return false;
        }

        // Point-sample the leading corner of the hit box in the direction of
        // travel. Good enough for the small, fast, mostly axis-aligned shots
        // this engine spawns; a stationary shot (vel == 0) samples its
        // bottom-right corner, which never matters since it can't be moving
        // into a wall in the first place.
        const double lead_x = pos.x + (vel.x >= 0.0 ? _hit_half_size.x : -_hit_half_size.x);
        const double lead_y = pos.y + (vel.y >= 0.0 ? _hit_half_size.y : -_hit_half_size.y);

        const TileAttribute attr = _terrain->AttributeAt(lead_x, lead_y);
        return systems::physics::Has(attr, TileAttribute::Solid) ||
            systems::physics::Has(attr, TileAttribute::Damage);
    }

    void ProjectileEntity::OnEntityCollision(IEntity& other)
    {
        // A deflector (Met hidden under its helmet) bounces the shot instead
        // of consuming it -- queried the same way a receiver queries
        // IAttackInfo. The deflector's own OnEntityCollision() independently
        // skips taking damage; this is purely this shot's own reaction.
        if (auto* deflector = dynamic_cast<systems::physics::IDeflector*>(&other);
            deflector != nullptr && deflector->DeflectsAttacks())
        {
            deflect_();
            return;
        }

        // Any collidable partner reaching here has already passed the CollisionMatrix
        // filter (Enemy/Trap for the ProjectilePlayer layer), so a single shot is
        // consumed on any qualifying hit -- matches the original Rock Buster behavior.
        Kill();
    }

    void ProjectileEntity::deflect_() noexcept
    {
        // Fixed upward-back arc, same speed as before -- a glancing "clink"
        // off the helmet rather than an aimed reflection back at whoever
        // fired it (see the design discussion this landed from: a redirect
        // reads as clearly "no effect" without turning the player's own shot
        // into a threat against them).
        constexpr double kDeflectAngleRad = -0.6; // ~-34 degrees; screen Y grows down, so negative = up.
        const double speed = std::hypot(vel.x, vel.y);
        const double reversed_dir = (vel.x >= 0.0) ? -1.0 : 1.0;
        vel.x = speed * reversed_dir * std::cos(kDeflectAngleRad);
        vel.y = speed * std::sin(kDeflectAngleRad);
        _pending_deflected = true;
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
