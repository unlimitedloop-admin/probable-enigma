//==============================================================================
// 
//  Project: mm2hack
//  IAnimTickable.h
// 
//  Interface for all animatable entities.
// 
//==============================================================================
#pragma once


namespace mm2hack::apps::world::entity
{
    // Interface for all animatable entities
    struct IAnimTickable
    {
        virtual ~IAnimTickable() = default;
        virtual void TickAnimation(double dt) = 0;
    };
}