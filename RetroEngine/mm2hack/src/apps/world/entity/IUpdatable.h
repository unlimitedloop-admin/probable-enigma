//==============================================================================
// 
//  Project: mm2hack
//  IUpdatable.h
// 
//  Interface for all updatable entities.
// 
//==============================================================================
#pragma once

#include "apps/systems/view/ViewState.h"

namespace mm2hack::apps::world::entity
{
    // Interface for updatable entities
    struct IUpdatable
    {
        virtual ~IUpdatable() = default;
        virtual void Update(const systems::view::ViewState* view, double dt) = 0;
    };
}