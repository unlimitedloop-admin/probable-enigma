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
        out.maxDistanceY = dy;

        TileAttribute a = TileAttribute::None;
        if (probeGround_(probes, a))
        {
            out.hit = true;
            out.maxDistanceY = 0.0;
            out.hitY = probes.bottomLine.middlePoint.y;
            out.attr = a;
        }

        TileAttribute b = TileAttribute::None;
        if (probeCeiling_(probes, b, 1.0))
        {
            out.hit = true;
            out.attr = b;
            out.hitY = probes.topLine.middlePoint.y;
            out.maxDistanceY = 0.0;
        }

        return out;
    }

    bool TileQueryService::IsGroundLike(const AvatarDirection /*direction*/, const Probes& probes, double dy) const
    {
        auto hit = SweepVertical(probes, dy);
        return hit.hit;
    }

    bool TileQueryService::IsLadderTop(const AvatarDirection direction, const Probes& probes, double speedY) const
    {
        return isLadderTopUnderfoot_(direction, probes, speedY);
    }

    SweepVHit TileQueryService::sweepDown_(const Probes& probes, double dy) const
    {
        SweepVHit out{};
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

        for (double x : xs)
        {
            const auto g = classifyGroundAt_(x, targetY, /*includeOneWay=*/true);
            if (g == TileAttribute::None)
            {
                continue;   // No reached ground here.
            }

            const int row = static_cast<int>(targetY) / _ts;
            const double topY = static_cast<double>(row * _ts);

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
}