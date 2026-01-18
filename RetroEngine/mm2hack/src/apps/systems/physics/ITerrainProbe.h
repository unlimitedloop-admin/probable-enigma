//==============================================================================
// 
//  Project: mm2hack
//  ITerrainProbe.h
// 
//  Terrain probing interface for detecting collisions and ground types.
// 
//==============================================================================
#pragma once

#include <cstdint>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "Probes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    using apps::world::entity::avatar::AvatarDirection;
    using foundation::math::Vec2;

    // Kind of vertical hit
    enum class VHitKind : std::uint8_t
    {
        None,
        Floor,
        Ceiling
    };

    // Result of a horizontal sweep operation
    struct SweepHHit
    {
        bool hit{ false };
        double maxDistanceX{ 0.0 };
        double hitX{ 0.0 };
        TileAttribute attr{ TileAttribute::None };
    };

    // Result of a vertical sweep operation
    struct SweepVHit
    {
        bool hit{ false };
        double maxDistanceY{ 0.0 };
        double hitY{ 0.0 };
        TileAttribute attr{ TileAttribute::None };
        VHitKind kind{ VHitKind::None };
    };

    // Result of an overlap fix operation on the X-axis
    struct OverlapXFix
    {
        bool hit{ false };
        double pushX{ 0.0 };    // Positive: push right, Negative: push left
    };


    // Terrain probing interface
    class ITerrainProbe
    {
    public:
        virtual ~ITerrainProbe() = default;
        virtual void SetCurrentPage(std::size_t pageIndex) noexcept = 0;
        // H-sweep: If moving dx, will it hit something?
        virtual SweepHHit SweepHorizontal(const Probes& probes, double dx, bool airFlag = false) const = 0;
        // V-sweep: If moving dy, will it hit something?
        virtual SweepVHit SweepVertical(const Probes& probes, Vec2 v) const = 0;
        // Returns true if the feet are considered "ground-like" (floor or ladder top special case)
        virtual bool IsGroundLike(const AvatarDirection direction, const Probes& probes, double dy) const = 0;
        // Resolve horizontal overlap with Solid tiles
        virtual OverlapXFix ResolveOverlapX(const Probes& p, const double parity) const = 0;

        virtual TileAttribute AttributeAt(Vec2 pos) const = 0;
        virtual TileAttribute AttributeAt(double worldX, double worldY) const = 0;
    };
}
