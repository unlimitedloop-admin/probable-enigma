//==============================================================================
// 
//  Project: mm2hack
//  EntityBase.h
// 
//  TODO: Add description.
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

        // Position and velocity
        Vec2 pos{};
        Vec2 vel{};

    protected:
        bool _alive{ true };
    };
}