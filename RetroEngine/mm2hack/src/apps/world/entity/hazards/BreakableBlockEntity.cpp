#include "pch.h"

#include "BreakableBlockEntity.h"

#include <cmath>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/bg/BGTileManager.h"
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
            writer.WriteI32(tile_index) &&
            health.Save(writer) &&
            writer.WriteF64(half_size.x) &&
            writer.WriteF64(half_size.y);
    }

    bool BreakableBlockEntityState::Load(core::save::StateReader& reader)
    {
        BreakableBlockEntityState loaded{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadI32(loaded.tile_index) ||
            !loaded.health.Load(reader) ||
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
            tile_index >= 0 && tile_index <= 65'535 &&
            health.IsValid() &&
            std::isfinite(half_size.x) && half_size.x > 0.0 && half_size.x <= kMaximumHalfSize &&
            std::isfinite(half_size.y) && half_size.y > 0.0 && half_size.y <= kMaximumHalfSize;
    }

    BreakableBlockEntity::BreakableBlockEntity(
        Vec2 spawn_pos,
        rendering::bg::BGTileManager::Id tileset_id,
        int tile_index,
        int max_hp,
        Vec2 half_size,
        DamageTable resistances)
        : _tileset_id(tileset_id), _tile_index(tile_index), _half(half_size),
          _health(max_hp, resistances)
    {
        pos = spawn_pos;
    }

    BreakableBlockEntity::BreakableBlockEntity(
        const BreakableBlockEntityState& state,
        rendering::bg::BGTileManager::Id tileset_id,
        DamageTable resistances)
        : _tileset_id(tileset_id)
    {
        _health = systems::combat::HealthComponent{ state.health.max_hp, resistances };
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
            .tile_index = _tile_index,
            .health = _health.CaptureState(),
            .half_size = _half,
        };
    }

    bool BreakableBlockEntity::RestoreState(const BreakableBlockEntityState& state) noexcept
    {
        if (!state.IsValid() || !RestoreKinematicState(state.kinematic) ||
            !_health.RestoreState(state.health, _health.Resistances()))
        {
            return false;
        }
        _tile_index = state.tile_index;
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
        res.GetBGTileManager().DrawTileById(_tileset_id, _tile_index, x, y);
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

        if (ApplyAttack(*attack))
        {
            // The destruction VFX/SFX itself is fired by
            // AbstractActionPhase::spawnDestructionEffectsForTheDead_() right after
            // this collision pass, keyed off IDamageable + IsAlive() -- nothing more
            // to do here than die.
            Kill();
        }
    }

    bool BreakableBlockEntity::ApplyAttack(const systems::physics::IAttackInfo& attack) noexcept
    {
        return _health.ApplyAttack(attack);
    }
}
