//==============================================================================
// 
//  Project: mm2hack
//  Camera.h
// 
//  Definition of the view camera in world.
// 
//==============================================================================
#pragma once

#include <cstdlib>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/ViewState.h"
#include "config/SystemConfig.h"

namespace mm2hack::apps::systems::scrolling::atomic
{
    // View camera representation
    struct Camera
    {
        using Scalar = systems::view::Scalar;
        using conf = config::SystemConfig;

        Scalar x{ 0 }, y{ 0 };
        int vw{ conf::kScreenWidth }, vh{ conf::kScreenHeight };

        static constexpr Scalar kCenterX = conf::kScreenWidth / 2.0;
        static constexpr Scalar kCenterY = conf::kScreenHeight / 2.0;

        static inline bool NearlyZero(Scalar v, Scalar eps = foundation::math::kEps) noexcept { return std::abs(v) <= eps; }
        static inline bool NearlyEqual(Scalar a, Scalar b, Scalar eps = foundation::math::kEps) noexcept { return std::abs(a - b) <= eps; }
    };
}