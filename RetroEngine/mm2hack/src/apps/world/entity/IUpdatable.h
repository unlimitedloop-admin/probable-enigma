//==============================================================================
// 
//  Project: mm2hack
//  IUpdatable.h
// 
//  Interface for all updatable entities.
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::world::entity
{
    // Interface for updatable entities
    struct IUpdatable
    {
        virtual ~IUpdatable() = default;
        virtual void Update(double dt) = 0;
    };
}