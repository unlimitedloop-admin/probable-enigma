//==============================================================================
//
//  Project: mm2hack
//  IAttackInfo.h
//
//  Interface for colliders that carry an attack payload on contact.
//
//==============================================================================
#pragma once

namespace mm2hack::apps::systems::physics
{
    // Implemented by colliders that deal damage on contact (player projectiles,
    // and later contact-damage enemies, etc.). Kept separate from ICollider so
    // that colliders with no attack payload (the player itself, static hazards,
    // items) are not forced to implement it; a receiver queries for this via
    // dynamic_cast from the IEntity passed to OnEntityCollision().
    struct IAttackInfo
    {
        virtual ~IAttackInfo() = default;

        // Damage/attack power carried by this collider.
        [[nodiscard]] virtual int AttackPower() const noexcept = 0;
    };
}
