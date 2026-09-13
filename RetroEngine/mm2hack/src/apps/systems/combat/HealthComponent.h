//==============================================================================
//
//  Project: mm2hack
//  HealthComponent.h
//
//  Reusable HP + per-weapon resistance tracking. Entities compose this rather
//  than inherit from it, and forward IDamageable calls to it -- shared by
//  breakable world objects today, and enemies once those land.
//
//==============================================================================
#pragma once

#include <algorithm>
#include <cstdint>

#include "DamageTable.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::systems::physics
{
    struct IAttackInfo;
}

namespace mm2hack::apps::systems::combat
{
    // Persisted half of HealthComponent. The resistance table itself is not
    // saved -- it is reconstructed from spawn parameters like every other
    // fixed trait (sprite id, half-size, ...), only current/max HP vary.
    struct HealthComponentState final
    {
        std::int32_t max_hp{ 1 };
        std::int32_t hp{ 1 };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

    class HealthComponent
    {
    public:
        HealthComponent() noexcept = default;
        HealthComponent(int max_hp, DamageTable resistances) noexcept
            : _max_hp(std::max(1, max_hp)), _hp(std::max(1, max_hp)), _table(resistances)
        {
        }

        // Applies an incoming attack through this component's resistance table.
        // Returns true only on the hit that brings HP down to <= 0; false for
        // every other hit (including ones that arrive after death, or that the
        // resistance table fully absorbs).
        bool ApplyAttack(const physics::IAttackInfo& attack) noexcept;

        [[nodiscard]] int CurrentHP() const noexcept { return _hp; }
        [[nodiscard]] int MaxHP() const noexcept { return _max_hp; }
        [[nodiscard]] bool IsDead() const noexcept { return _hp <= 0; }
        [[nodiscard]] const DamageTable& Resistances() const noexcept { return _table; }

        [[nodiscard]] HealthComponentState CaptureState() const noexcept;
        // `resistances` comes from the entity's own spawn parameters, not the save data.
        bool RestoreState(const HealthComponentState& state, DamageTable resistances) noexcept;

    private:
        int _max_hp{ 1 };
        int _hp{ 1 };
        DamageTable _table{ DamageTable::Neutral() };
    };
}
