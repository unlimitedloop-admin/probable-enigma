#include "pch.h"

#include "TileQueryService.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "ITerrainProbe.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    SweepHit TileQueryService::SweepVertical(const RectF& bounds, double dy) const
    {
        // W.I.P.: Vertical sweep implementation.
        return SweepHit();
    }

    bool TileQueryService::IsGroundLike(const RectF& bounds, double speedY) const
    {
        if (hasBlockUnderfoot(bounds, speedY))
        {
            return true;
        }
        if (isLadderTopUnderfoot(bounds, speedY))
        {
            return true;
        }
        return false;
    }

    bool TileQueryService::IsLadderTop(const RectF& bounds, double dy) const
    {
        // W.I.P.: Is at the top of a ladder?
        return false;
    }

    bool TileQueryService::hasBlockUnderfoot(const RectF& bounds, double speedY) const
    {
        // "Just below the feet" is the image. Even if speedY is 0, peek 1px below.
        const double probeY = bounds.bottom() + (speedY > 0.0 ? speedY : 1.0);

        // Here, a simplified version looks only at the center of the AABB.
        // If it's fixed at 32x32, it's okay to sample 3 points.
        const double midX = (bounds.left() + bounds.right()) * 0.5;

        const auto attr = attrAt(midX, probeY);
        return Has(attr, TileAttribute::Solid);
    }

    bool TileQueryService::isLadderTopUnderfoot(const RectF& bounds, double speedY) const
    {
        const double probeY = bounds.bottom() + (speedY > 0.0 ? speedY : 1.0);
        const double midX = (bounds.left() + bounds.right()) * 0.5;

        // On the tile below, is there a ladder?
        const auto below = attrAt(midX, probeY);
        if (!Has(below, TileAttribute::Ladder))
        {
            return false;
        }

        // Is the tile 1 above SPACE? (= top of the ladder)
        const double aboveY = probeY - _ts;
        const auto above = attrAt(midX, aboveY);

        const bool isSpace = Has(above, TileAttribute::Empty);
        return isSpace;
    }

    TileAttribute TileQueryService::attrAt(double worldX, double worldY) const
    {
        const int tx = static_cast<int>(worldX) / _ts;
        const int ty = static_cast<int>(worldY) / _ts;
        return _map.SampleTileAttribute(tx, ty);
    }
}