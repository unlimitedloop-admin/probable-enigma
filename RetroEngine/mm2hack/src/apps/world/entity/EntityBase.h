//==============================================================================
// 
//  Project: mm2hack
//  EntityBase.h
// 
//  Base class for all entities in the game.
// 
//==============================================================================
#pragma once

#include "IEntity.h"

#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::world::entity
{
    using foundation::math::Vec2;

    // Basic implementation of IEntity
    class EntityBase : public IEntity
    {
    public:
        bool IsAlive() const noexcept override { return _alive; }
        void Kill() noexcept override { _alive = false; }
        [[nodiscard]] EntityInstanceId StateInstanceId() const noexcept override { return _instance_id; }
        void AssignStateInstanceId(EntityInstanceId id) noexcept override { _instance_id = id; }

        // Position and velocity
        Vec2 pos{};
        Vec2 vel{};

    protected:
        bool _alive{ true };

    private:
        EntityInstanceId _instance_id{};
    };
}
