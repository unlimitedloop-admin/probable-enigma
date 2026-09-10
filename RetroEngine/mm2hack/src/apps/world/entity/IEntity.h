//==============================================================================
// 
//  Project: mm2hack
//  IEntity.h
// 
//  Interface for all entities in the game.
// 
//==============================================================================
#pragma once

#include <cstdint>

#include "core/save/StateIO.h"
#include "IRenderable.h"
#include "IUpdatable.h"

namespace mm2hack::apps::world::entity
{
    using EntityInstanceId = std::uint32_t;

    enum class EntityTypeId : std::uint16_t
    {
        Unknown = 0,
        Player = 1,
        Projectile = 2,
        ChargeEffect = 3,
        SlidingDustEffect = 4,
        SplashEffect = 5
    };

    // Base interface for all entities
    struct IEntity : IUpdatable, IRenderable
    {
        virtual bool IsAlive() const noexcept = 0;
        virtual void Kill() noexcept = 0;
        [[nodiscard]] virtual EntityTypeId StateTypeId() const noexcept = 0;
        [[nodiscard]] virtual EntityInstanceId StateInstanceId() const noexcept = 0;
        virtual void AssignStateInstanceId(EntityInstanceId id) noexcept = 0;
        [[nodiscard]] virtual std::uint16_t StateComponentVersion() const noexcept { return 0; }
        virtual bool SaveState(core::save::StateWriter& writer) const
        {
            (void)writer;
            return false;
        }
    };
}
