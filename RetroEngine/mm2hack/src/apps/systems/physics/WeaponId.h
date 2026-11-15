//==============================================================================
//
//  Project: mm2hack
//  WeaponId.h
//
//  Identifies which weapon an attack payload originated from, independent of
//  its power (charge level). Shared vocabulary between attack-emitting
//  colliders (IAttackInfo) and damage-receiving objects (combat::DamageTable).
//
//==============================================================================
#pragma once

#include <cstdint>

namespace mm2hack::apps::systems::physics
{
    // Weapon identity for damage-table lookups. A charged Rock Buster shot is
    // still WeaponId::Buster -- charge level changes IAttackInfo::AttackPower(),
    // not the weapon itself. New special weapons get their own entry here.
    enum class WeaponId : std::uint8_t
    {
        Buster = 0,
        // A generic enemy projectile. Resolves against PlayerEntity's own
        // combat::DamageTable (see PlayerEntity::ApplyAttack()).
        EnemyShot,
        // Touching an enemy's body directly (no projectile involved). Kept
        // distinct from EnemyShot so a kind's resistance to being touched can
        // differ from its resistance to being shot at, later.
        EnemyContact,

        Count
    };
}
