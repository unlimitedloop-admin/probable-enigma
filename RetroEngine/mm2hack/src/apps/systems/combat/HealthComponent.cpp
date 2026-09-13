#include "pch.h"

#include "HealthComponent.h"

#include "apps/systems/physics/IAttackInfo.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::systems::combat
{
    bool HealthComponentState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() &&
            writer.WriteI32(max_hp) &&
            writer.WriteI32(hp);
    }

    bool HealthComponentState::Load(core::save::StateReader& reader)
    {
        HealthComponentState loaded{};
        if (!reader.ReadI32(loaded.max_hp) || !reader.ReadI32(loaded.hp))
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

    bool HealthComponentState::IsValid() const noexcept
    {
        return max_hp >= 1 && max_hp <= 1'000 && hp >= 1 && hp <= max_hp;
    }

    bool HealthComponent::ApplyAttack(const physics::IAttackInfo& attack) noexcept
    {
        if (IsDead())
        {
            return false;
        }

        const int damage = _table.ComputeDamage(attack.Weapon(), attack.AttackPower());
        if (damage <= 0)
        {
            return false;
        }

        _hp -= damage;
        return IsDead();
    }

    HealthComponentState HealthComponent::CaptureState() const noexcept
    {
        return HealthComponentState{ _max_hp, _hp };
    }

    bool HealthComponent::RestoreState(const HealthComponentState& state, DamageTable resistances) noexcept
    {
        if (!state.IsValid())
        {
            return false;
        }
        _max_hp = state.max_hp;
        _hp = state.hp;
        _table = resistances;
        return true;
    }
}
