//==============================================================================
//
//  Project: mm2hack
//  IDamageable.h
//
//  Interface for entities that track HP and can be destroyed by attacks.
//
//==============================================================================
#pragma once

namespace mm2hack::apps::systems::physics
{
    struct IAttackInfo;
}

namespace mm2hack::apps::systems::combat
{
    // Implemented by entities with an HP pool that attacks can deplete (breakable
    // world objects today, enemies later). Kept separate from ICollider/IAttackInfo:
    // a collider only describes shape/layer, this describes HP lifecycle. Entities
    // typically implement this by composing and delegating to HealthComponent.
    struct IDamageable
    {
        virtual ~IDamageable() = default;

        // Applies an incoming attack, honoring this entity's own resistance
        // table. Returns true the moment HP reaches <= 0 (the entity just died);
        // returns false on every other hit, including ones after it already died.
        virtual bool ApplyAttack(const physics::IAttackInfo& attack) noexcept = 0;

        [[nodiscard]] virtual int CurrentHP() const noexcept = 0;
        [[nodiscard]] virtual int MaxHP() const noexcept = 0;
        [[nodiscard]] virtual bool IsDead() const noexcept = 0;
    };
}
