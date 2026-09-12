#include "pch.h"

#include "BreakableBlockEntity.h"

#include <cmath>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/systems/physics/IAttackInfo.h"
#include "apps/systems/view/RenderContext.h"
#include "apps/systems/view/ViewState.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::hazards
{
    using systems::view::Layer;
    using systems::view::RenderContext;
    using systems::view::ViewState;

    bool BreakableBlockEntityState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() && kinematic.Save(writer) &&
            writer.WriteI32(base_texture) &&
            writer.WriteI32(max_hp) &&
            writer.WriteI32(hp) &&
            writer.WriteF64(half_size.x) &&
            writer.WriteF64(half_size.y);
    }

    bool BreakableBlockEntityState::Load(core::save::StateReader& reader)
    {
        BreakableBlockEntityState loaded{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadI32(loaded.base_texture) ||
            !reader.ReadI32(loaded.max_hp) ||
            !reader.ReadI32(loaded.hp) ||
            !reader.ReadF64(loaded.half_size.x) ||
            !reader.ReadF64(loaded.half_size.y))
        {
            return false;
        }
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool BreakableBlockEntityState::IsValid() const noexcept
    {
        constexpr double kMaximumHalfSize = 256.0;
        return kinematic.IsValid() &&
            base_texture >= 0 && base_texture <= 65'535 &&
            max_hp >= 1 && max_hp <= 1'000 &&
            hp >= 1 && hp <= max_hp &&
            std::isfinite(half_size.x) && half_size.x > 0.0 && half_size.x <= kMaximumHalfSize &&
            std::isfinite(half_size.y) && half_size.y > 0.0 && half_size.y <= kMaximumHalfSize;
    }

    BreakableBlockEntity::BreakableBlockEntity(
        Vec2 spawn_pos,
        rendering::sprite::SpriteManager::Id sprite_id,
        int base_texture,
        int max_hp,
        Vec2 half_size)
        : _id(sprite_id), _base_texture(base_texture), _half(half_size),
          _max_hp(std::max(1, max_hp)), _hp(std::max(1, max_hp))
    {
        pos = spawn_pos;
    }

    BreakableBlockEntity::BreakableBlockEntity(
        const BreakableBlockEntityState& state,
        rendering::sprite::SpriteManager::Id sprite_id)
        : _id(sprite_id)
    {
        RestoreState(state);
    }

    bool BreakableBlockEntity::SaveState(core::save::StateWriter& writer) const
    {
        return CaptureState().Save(writer);
    }

    BreakableBlockEntityState BreakableBlockEntity::CaptureState() const noexcept
    {
        return BreakableBlockEntityState{
            .kinematic = CaptureKinematicState(),
            .base_texture = _base_texture,
            .max_hp = _max_hp,
            .hp = _hp,
            .half_size = _half,
        };
    }

    bool BreakableBlockEntity::RestoreState(const BreakableBlockEntityState& state) noexcept
    {
        if (!state.IsValid() || !RestoreKinematicState(state.kinematic))
        {
            return false;
        }
        _base_texture = state.base_texture;
        _max_hp = state.max_hp;
        _hp = state.hp;
        _half = state.half_size;
        return true;
    }

    Layer BreakableBlockEntity::DrawLayer() const noexcept
    {
        return Layer::Actors;
    }

    void BreakableBlockEntity::Update(const ViewState* view, double dt)
    {
        // Static object: nothing to simulate. Destruction happens via OnEntityCollision.
        (void)view;
        (void)dt;
    }

    void BreakableBlockEntity::Render(RenderContext& ctx)
    {
        if (!IsAlive() || ctx.view == nullptr)
        {
            return;
        }

        const auto& view = *ctx.view;
        const int x = static_cast<int>(pos.x - view.viewWorldX - _half.x);
        const int y = static_cast<int>(pos.y - view.viewWorldY - _half.y);

        auto& res = runtime::GameContext::GetInstance().GetResourceManager();
        res.GetSpriteManager().UseById(_id, _base_texture, x, y);
    }

    BreakableBlockEntity::RectF BreakableBlockEntity::Bounds() const
    {
        return { pos.x - _half.x, pos.y - _half.y, _half.x * 2.0, _half.y * 2.0 };
    }

    void BreakableBlockEntity::OnEntityCollision(IEntity& other)
    {
        if (!IsAlive())
        {
            return;
        }

        // Only a collider that carries an attack payload damages the block (a
        // player projectile today; a contact-damage enemy could implement
        // IAttackInfo too, later). Plain contact -- e.g. the player just
        // standing against it -- does nothing yet.
        auto* attack = dynamic_cast<systems::physics::IAttackInfo*>(&other);
        if (attack == nullptr)
        {
            return;
        }

        _hp -= attack->AttackPower();
        if (_hp <= 0)
        {
            // TODO: spawn a destruction effect once one exists.
            Kill();
        }
    }
}
