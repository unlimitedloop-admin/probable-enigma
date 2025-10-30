//==============================================================================
// 
//  Project: mm2hack
//  EntityBase.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

#include "IEntity.h"

#include "apps/mod/CoordinateTypes.h"

namespace mm2hack::apps::graphics::entity
{
    using mod::Vec2;

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