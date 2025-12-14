#include "pch.h"

#include "TileQueryService.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "ITerrainProbe.h"
#include "Probes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    namespace
    {
        // Helper to get array size at compile time
        template <std::size_t N, typename T>
        constexpr std::size_t array_size(const T(&)[N]) noexcept
        {
            return N;
        }
    }

    SweepHHit TileQueryService::SweepHorizontal(const Probes& probes, double dx, bool airFlag) const
    {
        if (dx > 0.0)
        {
            return sweepRight_(probes, dx, airFlag);
        }
        if (dx < 0.0)
        {
            return sweepLeft_(probes, dx, airFlag);
        }

        SweepHHit out{};
        out.maxDistanceX = 0.0;
        return out;
    }

    SweepVHit TileQueryService::SweepVertical(const Probes& probes, double dy) const
    {
        if (dy > 0.0)
        {
            // Moving down
            return sweepDown_(probes, dy);
        }

        if (dy < 0.0)
        {
            // Moving up
            return sweepUp_(probes, dy);
        }

        SweepVHit out{};
        out.kind = VHitKind::Floor;
        out.maxDistanceY = dy;

        TileAttribute b = TileAttribute::None;
        if (probeCeiling_(probes, b, 1.0))
        {
            out.hit = false;
            out.maxDistanceY = 0.0;
            out.hitY = probes.topLine.middlePoint.y;
            out.attr = b;
        }

        TileAttribute a = TileAttribute::None;
        if (probeGround_(probes, a))
        {
            out.hit = true;
            out.maxDistanceY = 0.0;
            out.hitY = probes.bottomLine.middlePoint.y;
            out.attr = a;
        }

        return out;
    }

    bool TileQueryService::IsGroundLike(const AvatarDirection /*direction*/, const Probes& probes, double dy) const
    {
        if (dy <= 0.0)
        {
            return false;
        }

        auto hit = SweepVertical(probes, dy);
        return hit.hit;
    }

    bool TileQueryService::IsLadderTop(const AvatarDirection direction, const Probes& probes, double speedY) const
    {
        return isLadderTopUnderfoot_(direction, probes, speedY);
    }

    bool TileQueryService::probeWall_(const Probes& probes, TileAttribute& outAttr, double peekX) const
    {
        outAttr = TileAttribute::None;

        const double rightProbeX = probes.frontLine.middlePoint.x + std::max(0.0, peekX);
        const double leftProbeX = probes.rearLine.middlePoint.x - std::max(0.0, peekX);

        const double ysRight[] = {
            probes.frontLine.topPoint.y,
            probes.frontLine.middlePoint.y,
            probes.frontLine.bottomPoint.y
        };

        const double ysLeft[] = {
            probes.rearLine.topPoint.y,
            probes.rearLine.middlePoint.y,
            probes.rearLine.bottomPoint.y
        };

        for (double y : ysRight)
        {
            const TileAttribute a = classifyWallAt_(rightProbeX, y);
            if (a != TileAttribute::None)
            {
                outAttr = a;
                return true;
            }
        }

        for (double y : ysLeft)
        {
            const TileAttribute a = classifyWallAt_(leftProbeX, y);
            if (a != TileAttribute::None)
            {
                outAttr = a;
                return true;
            }
        }

        return false;
    }

    SweepHHit TileQueryService::sweepRight_(const Probes& probes, double dx, bool airFlag) const
    {
        SweepHHit out{};
        out.maxDistanceX = dx;

        if (dx <= 0.0)
        {
            return out;
        }

        const double curSideX = probes.frontLine.middlePoint.x;
        const double targetSideX = curSideX + dx;
        const double point3 = airFlag ? probes.frontLine.bottomPoint2.y : probes.frontLine.bottomPoint.y;

        const double ys[] = {
            probes.frontLine.topPoint.y,
            probes.frontLine.middlePoint.y,
            point3
        };

        bool found = false;
        double bestDist = dx;
        double bestHitX = curSideX + dx;
        TileAttribute bestAttr = TileAttribute::None;

        for (double py : ys)
        {
            const TileAttribute a = classifyWallAt_(targetSideX, py);
            if (a == TileAttribute::None)
            {
                continue;
            }

            // Tile column at destination x
            const int col = static_cast<int>(targetSideX) / _ts;

            // When moving right, we must stop at the tile's LEFT edge: col * ts
            const double wallLeftX = static_cast<double>(col * _ts);

            // Distance so that curSideX + dist == wallLeftX
            double dist = wallLeftX - curSideX; // should be >= 0
            if (dist < 0.0) dist = 0.0;
            if (dist > dx)  dist = dx;

            if (!found || dist < bestDist)
            {
                found = true;
                bestDist = dist;
                bestHitX = curSideX + dist;
                bestAttr = a;
            }
        }

        if (found)
        {
            out.hit = true;
            out.maxDistanceX = bestDist;
            out.hitX = bestHitX;
            out.attr = bestAttr;
        }

        return out;
    }

    SweepHHit TileQueryService::sweepLeft_(const Probes& probes, double dx, bool airFlag) const
    {
        SweepHHit out{};
        out.maxDistanceX = dx;

        if (dx >= 0.0)
        {
            return out;
        }

        const double curSideX = probes.frontLine.middlePoint.x;
        const double targetSideX = curSideX + dx; // dx is negative
        const double point3 = airFlag ? probes.frontLine.bottomPoint2.y : probes.frontLine.bottomPoint.y;

        const double ys[] = {
            probes.frontLine.topPoint.y,
            probes.frontLine.middlePoint.y,
            point3
        };

        bool found = false;
        double bestDist = dx;                 // negative; best is closer to 0 => larger value
        double bestHitX = curSideX + dx;
        TileAttribute bestAttr = TileAttribute::None;

        for (double py : ys)
        {
            const TileAttribute a = classifyWallAt_(targetSideX, py);
            if (a == TileAttribute::None)
            {
                continue;
            }

            const int col = static_cast<int>(targetSideX) / _ts;

            // When moving left, stop at the tile's RIGHT edge: (col + 1) * ts
            const double wallRightX = static_cast<double>((col + 1) * _ts);

            // Distance so that curSideX + dist == wallRightX
            double dist = wallRightX - curSideX; // should be <= 0
            if (dist > 0.0) dist = 0.0;
            if (dist < dx)  dist = dx;

            // Choose the closest collision (largest dist, since dx is negative)
            if (!found || dist > bestDist)
            {
                found = true;
                bestDist = dist;
                bestHitX = curSideX + dist;
                bestAttr = a;
            }
        }

        if (found)
        {
            out.hit = true;
            out.maxDistanceX = bestDist;
            out.hitX = bestHitX;
            out.attr = bestAttr;
        }

        return out;
    }

    bool TileQueryService::probeGround_(const Probes& probes, TileAttribute& outAttr) const
    {
        constexpr double kPeek = 1.0;

        const double probeY = probes.bottomLine.middlePoint.y + kPeek;
        const double xs[] = {
            probes.bottomLine.frontPoint.x,
            probes.bottomLine.middlePoint.x,
            probes.bottomLine.behindPoint.x
        };

        TileAttribute best = TileAttribute::None;

        for (double x : xs)
        {
            const auto a = classifyGroundAt_(x, probeY, /*includeOneWay=*/true);
            if (Has(a, TileAttribute::Solid))
            {
                outAttr = TileAttribute::Solid;
                return true;
            }
            if (Has(a, TileAttribute::Ladder))
            {
                best = TileAttribute::Ladder;
            }
        }

        outAttr = best;
        return best != TileAttribute::None;
    }

    SweepVHit TileQueryService::sweepDown_(const Probes& probes, double dy) const
    {
        SweepVHit out{};
        out.kind = VHitKind::Floor;
        out.maxDistanceY = dy;

        if (dy <= 0.0) return out;

        const double curBottomY = probes.bottomLine.middlePoint.y;
        const double targetY = curBottomY + dy;

        const double xs[] = {
            probes.bottomLine.frontPoint.x,
            probes.bottomLine.middlePoint.x,
            probes.bottomLine.behindPoint.x
        };

        bool found = false;
        double bestDist = dy;
        TileAttribute bestAttr = TileAttribute::None;

        const int row = static_cast<int>(targetY) / _ts;
        const double topY = static_cast<double>(row * _ts);

        for (double x : xs)
        {
            const auto g = classifyGroundAt_(x, targetY, /*includeOneWay=*/true);
            if (g == TileAttribute::None)
            {
                continue;   // No reached ground here.
            }

            double dist = topY - curBottomY;
            if (dist < 0.0) dist = 0.0;
            if (dist > dy)  dist = dy;

            if (!found || dist < bestDist)
            {
                found = true;
                bestDist = dist;
                bestAttr = g;
            }
        }

        if (found)
        {
            out.hit = true;
            out.maxDistanceY = bestDist;
            out.hitY = curBottomY + bestDist; // will be on top of the ground
            out.attr = bestAttr;
        }
        return out;
    }

    bool TileQueryService::hasBlockUnderfoot_(const AvatarDirection direction, const Probes& probes, double dy) const
    {
        // Check just below the bottom, or 1 pixel below if not moving down.
        const double probeY = probes.bottomLine.middlePoint.y + (dy > 0.0 ? dy : 1.0);

        double xs[3]{};
        collectBottomSampleXs_(direction, probes, xs);

        for (double px : xs)
        {
            const TileAttribute attr = classifyGroundAt_(px, probeY, /*includeOneWay=*/false);
            if (Has(attr, TileAttribute::Solid) && !Has(attr, TileAttribute::NoCollision))
            {
                return true;
            }
        }
        return false;
    }

    void TileQueryService::collectBottomSampleXs_(const AvatarDirection direction, const Probes& probes, double(&outXs)[3]) const noexcept
    {
        const double midX = probes.bottomLine.middlePoint.x;
        const double frontX = probes.bottomLine.frontPoint.x;
        const double behindX = probes.bottomLine.behindPoint.x;

        if (direction == AvatarDirection::Left)
        {
            outXs[0] = frontX;
            outXs[1] = midX;
            outXs[2] = behindX;
        }
        else
        {
            // Caution: The points vary based on direction. 0 and 2.
            outXs[0] = behindX;
            outXs[1] = midX;
            outXs[2] = frontX;
        }
    }

    bool TileQueryService::isLadderTopUnderfoot_(const AvatarDirection direction, const Probes& probes, double dy) const
    {
        const double probeY = probes.bottomLine.middlePoint.y + (dy > 0.0 ? dy : 1.0);

        double xs[3]{};
        collectBottomSampleXs_(direction, probes, xs);

        for (double px : xs)
        {
            const TileAttribute attr = classifyGroundAt_(px, probeY, /*includeOneWay=*/false);
            if (Has(attr, TileAttribute::Ladder) && !Has(attr, TileAttribute::NoCollision))
            {
                return true;
            }
        }
        return false;
    }

    bool TileQueryService::probeCeiling_(const Probes& probes, TileAttribute& outAttr, double peekY) const
    {
        outAttr = TileAttribute::None;

        const double probeY = probes.topLine.middlePoint.y - std::max(0.0, peekY);

        const double xs[] = {
            probes.topLine.frontPoint.x,
            probes.topLine.middlePoint.x,
            probes.topLine.behindPoint.x
        };

        for (double x : xs)
        {
            const TileAttribute a = classifyCeilingAt_(x, probeY);
            if (a != TileAttribute::None)
            {
                outAttr = a;
                return true;
            }
        }
        return false;
    }

    SweepVHit TileQueryService::sweepUp_(const Probes& probes, double dy) const
    {
        SweepVHit out{};
        out.kind = VHitKind::Ceiling;
        out.maxDistanceY = dy;

        if (dy >= 0.0)
        {
            return out;
        }

        // Current head line (Y) and target head line (Y) after moving dy.
        const double curTopY = probes.topLine.middlePoint.y;
        const double targetTopY = curTopY + dy; // dy is negative

        const double xs[] = {
            probes.topLine.frontPoint.x,
            probes.topLine.middlePoint.x,
            probes.topLine.behindPoint.x
        };

        bool found = false;
        double bestDist = dy;                 // dy is negative; bestDist should be closer to 0 (greater value)
        double bestHitY = curTopY + dy;
        TileAttribute bestAttr = TileAttribute::None;

        // We sample at the destination; for very large |dy| you may later replace this with row-walk sweep.
        for (double x : xs)
        {
            const TileAttribute hitAttr = classifyCeilingAt_(x, targetTopY);
            if (hitAttr == TileAttribute::None)
            {
                continue;
            }

            // The ceiling tile row at targetTopY
            const int row = static_cast<int>(targetTopY) / _ts;

            // We hit the tile from below, so we want the head to land on the tile's bottom edge:
            // bottomY = (row + 1) * tileSize
            const double ceilingBottomY = static_cast<double>((row + 1) * _ts);

            // Distance to move so that curTopY + dist == ceilingBottomY
            double dist = ceilingBottomY - curTopY; // should be negative or 0
            if (dist > 0.0) dist = 0.0;
            if (dist < dy) dist = dy;

            // Choose the "closest" collision (largest dist, since dy is negative)
            if (!found || dist > bestDist)
            {
                found = true;
                bestDist = dist;
                bestHitY = curTopY + dist;
                bestAttr = hitAttr;
            }
        }

        if (found)
        {
            out.hit = true;
            out.maxDistanceY = bestDist;
            out.hitY = bestHitY;
            out.attr = bestAttr;
        }

        return out;
    }

    TileAttribute TileQueryService::attrAt_(double worldX, double worldY) const
    {
        const int tx = static_cast<int>(worldX) / _ts;
        const int ty = static_cast<int>(worldY) / _ts;
        return _map.SampleTileAttribute(tx, ty);
    }

    TileAttribute TileQueryService::classifyGroundAt_(double x, double probeY, bool includeOneWay) const
    {
        const auto below = attrAt_(x, probeY);

        if (Has(below, TileAttribute::Solid) ||
            (includeOneWay && Has(below, TileAttribute::OneWayPlatform)))
        {
            return TileAttribute::Solid; // OnGround
        }

        // Can also land on the top of a ladder.
        if (Has(below, TileAttribute::Ladder))
        {
            const auto above = attrAt_(x, probeY - static_cast<double>(_ts));
            if (Has(above, TileAttribute::Empty))
            {
                return TileAttribute::Ladder; // OnGround [LadderTop]
            }
        }

        return TileAttribute::None;
    }

    TileAttribute TileQueryService::classifyCeilingAt_(double x, double probeY) const
    {
        const TileAttribute a = attrAt_(x, probeY);

        if (Has(a, TileAttribute::Solid))
        {
            return TileAttribute::Solid;
        }
        return TileAttribute::None;
    }

    TileAttribute TileQueryService::classifyWallAt_(double x, double y) const
    {
        const TileAttribute a = attrAt_(x, y);
        if (Has(a, TileAttribute::Solid))
        {
            return TileAttribute::Solid;
        }
        return TileAttribute::None;
    }
}