//==============================================================================
//
//  Project: mm2hack
//  DamageTable.h
//
//  Per-weapon resistance table: how much of an attack's power actually gets
//  through to a damageable object's HP. Mirrors the classic per-weapon
//  damage tables enemies carry, but expressed as a percentage of the
//  attacker's own power rather than a fixed HP value, so charge levels
//  (which change IAttackInfo::AttackPower()) still scale naturally.
//
//==============================================================================
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>

#include "apps/systems/physics/WeaponId.h"

namespace mm2hack::apps::systems::combat
{
    using physics::WeaponId;

    // Resistance table indexed by WeaponId. Every entry defaults to 100
    // (full damage) unless explicitly tuned. 0 means immune to that weapon;
    // values above 100 express a weak point (extra damage).
    class DamageTable
    {
    public:
        static constexpr int kImmunePercent{ 0 };
        static constexpr int kNormalPercent{ 100 };
        static constexpr int kMaxPercent{ 800 };

        constexpr DamageTable() noexcept
        {
            _percent.fill(kNormalPercent);
        }

        // Sets the resistance percentage for a single weapon.
        constexpr void SetPercent(WeaponId weapon, int percent) noexcept
        {
            _percent[Index(weapon)] = std::clamp(percent, kImmunePercent, kMaxPercent);
        }

        // Convenience: this weapon deals no damage at all.
        constexpr void SetImmune(WeaponId weapon) noexcept
        {
            SetPercent(weapon, kImmunePercent);
        }

        [[nodiscard]] constexpr int PercentFor(WeaponId weapon) const noexcept
        {
            return _percent[Index(weapon)];
        }

        // Resolves how much HP an attack of the given power/weapon actually
        // removes: percent-scaled, but never zero unless the weapon is
        // explicitly immune (a 1-power hit at 50% still deals 1, not 0).
        [[nodiscard]] constexpr int ComputeDamage(WeaponId weapon, int attack_power) const noexcept
        {
            const int percent = PercentFor(weapon);
            if (percent <= kImmunePercent || attack_power <= 0)
            {
                return 0;
            }

            const int scaled = (attack_power * percent) / kNormalPercent;
            return std::max(scaled, 1);
        }

        // A table where every weapon deals full, unscaled damage -- the
        // default for objects that have no special resistances yet.
        [[nodiscard]] static constexpr DamageTable Neutral() noexcept
        {
            return DamageTable{};
        }

        // A table where every weapon is immune -- HP can never move, i.e. the
        // object is effectively invincible regardless of its own max HP.
        [[nodiscard]] static constexpr DamageTable Invincible() noexcept
        {
            DamageTable table{};
            for (std::size_t i = 0; i < table._percent.size(); ++i)
            {
                table._percent[i] = kImmunePercent;
            }
            return table;
        }

    private:
        [[nodiscard]] static constexpr std::size_t Index(WeaponId weapon) noexcept
        {
            return static_cast<std::size_t>(weapon);
        }

        std::array<int, static_cast<std::size_t>(WeaponId::Count)> _percent{};
    };
}
