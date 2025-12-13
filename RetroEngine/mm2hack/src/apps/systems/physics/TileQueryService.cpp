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
        return out;
    }

    bool TileQueryService::IsGroundLike(const AvatarDirection /*direction*/, const Probes& probes, double dy) const
    {
        auto hit = SweepVertical(probes, dy);
        return hit.hit;
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

    TileAttribute TileQueryService::attrAt_(double worldX, double worldY) const
    {
        const int tx = static_cast<int>(worldX) / _ts;
        const int ty = static_cast<int>(worldY) / _ts;
        return _map.SampleTileAttribute(tx, ty);
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
}