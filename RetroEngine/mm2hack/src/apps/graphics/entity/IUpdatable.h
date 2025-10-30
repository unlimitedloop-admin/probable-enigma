//==============================================================================
// 
//  Project: mm2hack
//  IUpdatable.h
// 
//  ** Descriptions **
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::graphics::entity
{
    // Interface for updatable entities
    struct IUpdatable
    {
        virtual ~IUpdatable() = default;
        virtual void Update(double dt) = 0;
    };
}