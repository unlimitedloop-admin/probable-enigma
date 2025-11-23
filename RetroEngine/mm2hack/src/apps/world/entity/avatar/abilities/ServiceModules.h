//==============================================================================
// 
//  Project: mm2hack
//  ServiceModules.h
// 
//  Use inline functions to implement avatar abilities.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::world::entity::avatar::abilities
{
    using foundation::math::Vec2;

    // Interface for ladder service
    struct ILadderService
    {
        virtual ~ILadderService() = default;
        virtual bool CanGrabAt(const Vec2& worldPos) const = 0;
    };
}