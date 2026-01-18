//==============================================================================
// 
//  Project: mm2hack
//  IEntity.h
// 
//  Interface for all entities in the game.
// 
//==============================================================================
#pragma once

#include "IRenderable.h"
#include "IUpdatable.h"

namespace mm2hack::apps::world::entity
{
    // Base interface for all entities
    struct IEntity : IUpdatable, IRenderable
    {
        virtual bool IsAlive() const noexcept = 0;
        virtual void Kill() noexcept = 0;
    };
}