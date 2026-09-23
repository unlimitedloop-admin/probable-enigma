//==============================================================================
//
//  Project: mm2hack
//  IDeflector.h
//
//  Interface for colliders that deflect incoming attacks instead of taking
//  damage from them.
//
//==============================================================================
#pragma once

namespace mm2hack::apps::systems::physics
{
    // Implemented by colliders that can turn away an attack rather than
    // absorb it (Met hidden under its helmet today). Kept separate from
    // ICollider/IDamageable the same way IAttackInfo is -- an attacking
    // collider queries for this via dynamic_cast from the IEntity passed to
    // its own OnEntityCollision(), the moment it decides whether to apply
    // damage or bounce off.
    struct IDeflector
    {
        virtual ~IDeflector() = default;

        // Whether this collider deflects an incoming attack right now (not a
        // fixed trait -- e.g. Met only deflects while hidden under its
        // helmet, not once it rises to attack).
        [[nodiscard]] virtual bool DeflectsAttacks() const noexcept = 0;
    };
}
