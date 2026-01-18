//==============================================================================
// 
//  Project: mm2hack
//  ServiceModules.h
// 
//  Use inline functions to implement avatar abilities.
// 
//==============================================================================
#pragma once

#include <optional>
#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::systems::physics
{
    using foundation::math::Vec2;

    enum class LadderEntryKind
    {
        None,
        FromTopDown,    // Entering ladder from the top, going down
        NormalGrab,
    };

    // Interface for ladder service
    class ILadderService
    {
    public:
        virtual ~ILadderService() = default;

        // Simple: "can grab ladder at worldPos?"
        virtual bool CanGrabAt(const Vec2& worldPos) const = 0;

        // ladder center (world coordinate). nullopt if not ladder.
        virtual std::optional<Vec2> TryGetCenterXAt(const Vec2& worldPos) const = 0;

        virtual void setEntryKind(LadderEntryKind v) noexcept = 0;
        virtual LadderEntryKind getEntryKind() const noexcept = 0;
    };
}