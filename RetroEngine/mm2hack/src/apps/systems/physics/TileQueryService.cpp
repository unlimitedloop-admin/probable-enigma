#include "pch.h"

#include "TileQueryService.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "ITerrainProbe.h"
#include "Probes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    SweepHit TileQueryService::SweepVertical(const Probes& probes, double dy) const
    {
        // W.I.P.: Vertical sweep implementation.
        return SweepHit();
    }

    bool TileQueryService::IsGroundLike(const AvatarDirection direction, const Probes& probes, double speedY) const
    {
        if (hasBlockUnderfoot(direction, probes, speedY))
        {
            return true;
        }
        if (isLadderTopUnderfoot(direction, probes, speedY))
        {
            return true;
        }
        return false;
    }

    bool TileQueryService::IsLadderTop(const AvatarDirection direction, const Probes& probes, double dy) const
    {
        // W.I.P.: Is at the top of a ladder?
        return false;
    }

    bool TileQueryService::hasBlockUnderfoot(const AvatarDirection direction, const Probes& probes, double speedY) const
    {
        // "Just below the feet" is the image. Even if speedY is 0, peek 1px below.
        const double probeY  = probes.bottomLine.middlePoint.y + (speedY > 0.0 ? speedY : 1.0);
        const double midX    = probes.bottomLine.middlePoint.x;
        const double frontX  = probes.bottomLine.frontPoint.x;
        const double behindX = probes.bottomLine.behindPoint.x;

        const auto midAttr   = attrAt(midX, probeY);
        const auto leftAttr  = attrAt(direction == AvatarDirection::Left ? frontX : behindX, probeY);
        const auto rightAttr = attrAt(direction == AvatarDirection::Right ? frontX : behindX, probeY);
        return Has(midAttr, TileAttribute::Solid) || Has(leftAttr, TileAttribute::Solid) || Has(rightAttr, TileAttribute::Solid);
    }

    bool TileQueryService::isLadderTopUnderfoot(const AvatarDirection direction, const Probes& probes, double speedY) const
    {
        const double probeY = probes.bottomLine.middlePoint.y + (speedY > 0.0 ? speedY : 1.0);
        const double midX = probes.bottomLine.middlePoint.x;
        const double frontX = probes.bottomLine.frontPoint.x;
        const double behindX = probes.bottomLine.behindPoint.x;

        // On the tile below, is there a ladder?
        const auto midBelow = attrAt(midX, probeY);
        const auto leftBelow = attrAt(direction == AvatarDirection::Left ? frontX : behindX, probeY);
        const auto rightBelow = attrAt(direction == AvatarDirection::Right ? frontX : behindX, probeY);
        if (!Has(midBelow, TileAttribute::Ladder) && !Has(leftBelow, TileAttribute::Ladder) && !Has(rightBelow, TileAttribute::Ladder))
        {
            return false;
        }

        // Is the tile 1 above SPACE? (= top of the ladder)
        const double aboveY = probeY - _ts;
        const auto midAttr = attrAt(midX, aboveY);
        const auto leftAttr = attrAt(direction == AvatarDirection::Left ? frontX : behindX, aboveY);
        const auto rightAttr = attrAt(direction == AvatarDirection::Right ? frontX : behindX, aboveY);

        const bool isSpace = Has(midAttr, TileAttribute::Empty) && Has(leftAttr, TileAttribute::Empty) && Has(rightAttr, TileAttribute::Empty);
        return isSpace;
    }

    TileAttribute TileQueryService::attrAt(double worldX, double worldY) const
    {
        const int tx = static_cast<int>(worldX) / _ts;
        const int ty = static_cast<int>(worldY) / _ts;
        return _map.SampleTileAttribute(tx, ty);
    }
}