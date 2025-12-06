//==============================================================================
// 
//  Project: mm2hack
//  ITerrainProbe.h
// 
//  Terrain probing interface for detecting collisions and ground types.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "Probes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    using foundation::math::Vec2;
    using apps::world::entity::avatar::AvatarDirection;

    // Result of a sweep operation
    struct SweepHit
    {
        bool hit{ false };
        double snap{ 0.0 };          // Just in case of hit, how much to snap
        TileAttribute attr{ TileAttribute::None };
    };

    // Terrain probing interface
    class ITerrainProbe
    {
    public:
        virtual ~ITerrainProbe() = default;
        // V-sweep: If moving dy, will it hit something?
        virtual SweepHit SweepVertical(const Probes& probes, double dy) const = 0;
        // Returns true if the feet are considered "ground-like" (floor or ladder top special case)
        virtual bool IsGroundLike(const AvatarDirection direction, const Probes& probes, double dy) const = 0;
        // Returns true if special handling for being at the "top" of a ladder is needed
        virtual bool IsLadderTop(const AvatarDirection direction, const Probes& probes, double dy) const = 0;
    };
}
