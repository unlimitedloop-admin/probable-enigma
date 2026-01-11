#include "pch.h"

#include "TileQueryService.h"

#include <cmath>
#include <cstdlib>
#include <limits>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/stage/RoomGraphAdapter.h"
#include "ITerrainProbe.h"
#include "PageGridIndex.h"
#include "Probes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    using config::SystemConfig;

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

    SweepVHit TileQueryService::SweepVertical(const Probes& probes, Vec2 v) const
    {
        if (v.y > 0.0)
        {
            return sweepDown_(probes, v.x, v.y);
        }

        if (v.y < 0.0)
        {
            return sweepUp_(probes, v.x, v.y);
        }

        // v.y == 0 : apex / idle. Do NOT snap to ladder/one-way top.
        SweepVHit out{};
        out.kind = VHitKind::Floor;
        out.maxDistanceY = 0.0;

        TileAttribute a = TileAttribute::None;

        // No prevFootY => CanLandOnTopSurface() returns false => no one-frame grounding.
        if (probeGround_(probes, a, /*includeOneWay=*/false, /*prevFootY=*/std::nullopt))
        {
            out.hit = true;
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

        auto hit = SweepVertical(probes, Vec2{ 0.0, dy });
        return hit.hit;
    }

    OverlapXFix TileQueryService::ResolveOverlapX(const Probes& p, const double parity) const
    {
        // Small epsilon to stay strictly outside the solid tile
        double kEps = parity; // 1/256

        OverlapXFix out{};

        // Check whether a world position is inside a Solid tile
        const auto isSolid = [&](double x, double y) noexcept -> bool
            {
                return Has(attrAt_(x, y), TileAttribute::Solid);
            };

        // Check three probe points (top/middle/bottom) on a vertical line
        const auto insideLine3 = [&](double x, const auto& line) noexcept -> bool
            {
                return isSolid(x, line.topPoint.y) || isSolid(x, line.middlePoint.y) || isSolid(x, line.bottomPoint.y);
            };

        // Verify that after applying dx, BOTH front and rear probes are clear
        const auto clearsAll = [&](double dx) noexcept -> bool
            {
                const double fx = p.frontLine.middlePoint.x + dx;
                const double rx = p.rearLine.middlePoint.x + dx;

                const bool f = isSolid(fx, p.frontLine.topPoint.y) || isSolid(fx, p.frontLine.middlePoint.y) || isSolid(fx, p.frontLine.bottomPoint.y);

                const bool r = isSolid(rx, p.rearLine.topPoint.y) || isSolid(rx, p.rearLine.middlePoint.y) || isSolid(rx, p.rearLine.bottomPoint.y);

                return !(f || r);
            };

        // Select the candidate with the smallest absolute displacement
        auto best = [&](double curBest, double cand) noexcept -> double
            {
                return (std::abs(cand) < std::abs(curBest)) ? cand : curBest;
            };

        bool any = false;
        double bestDx = 0.0;

        // Add push candidates for a single probe X position
        const auto addCandidatesForX = [&](double x) noexcept
            {
                if (!any) { bestDx = std::numeric_limits<double>::infinity(); any = true; }

                // Determine the tile column this point belongs to
                const int col = static_cast<int>(x) / _ts;

                // Tile boundaries
                const double tileLeft = static_cast<double>(col * _ts);
                const double tileRight = static_cast<double>((col + 1) * _ts);

                // Candidate pushes:
                // - move left to just before the tile
                // - move right to just after the tile
                const double toLeft = (tileLeft - kEps) - x; // usually negative
                const double toRight = (tileRight + kEps) - x; // usually positive

                // Only accept candidates that fully clear both sides
                if (clearsAll(toLeft)) { bestDx = best(bestDx, toLeft); }
                if (clearsAll(toRight)) { bestDx = best(bestDx, toRight); }
            };

        // Detect horizontal penetration on each side
        const bool frontInside = insideLine3(p.frontLine.middlePoint.x, p.frontLine);
        const bool rearInside = insideLine3(p.rearLine.middlePoint.x, p.rearLine);

        if (!frontInside && !rearInside)
        {
            return out;
        }

        if (frontInside) { addCandidatesForX(p.frontLine.middlePoint.x); }
        if (rearInside) { addCandidatesForX(p.rearLine.middlePoint.x); }

        if (!any || !std::isfinite(bestDx))
        {
            return out; // No safe correction found
        }

        // Safety clamp:
        // Horizontal correction should never exceed one tile width
        if (std::abs(bestDx) > static_cast<double>(_ts) + kEps)
        {
            return out;
        }

        out.hit = true;
        out.pushX = bestDx;
        return out;
    }

    TileAttribute TileQueryService::AttributeAt(Vec2 pos) const
    {
        return AttributeAt(pos.x, pos.y);
    }

    TileAttribute TileQueryService::AttributeAt(double worldX, double worldY) const
    {
        return attrAt_(worldX, worldY);
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
        double bestDist = dx;   // negative; best is closer to 0 => larger value
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

    bool TileQueryService::probeGround_(const Probes& probes, TileAttribute& outAttr, bool includeOneWay, std::optional<double> prevFootY) const
    {
        constexpr double kPeek = 1.0;

        const double curBottomY = probes.bottomLine.middlePoint.y;
        const double probeY = curBottomY + kPeek;

        const double xs[] = {
            probes.bottomLine.frontPoint.x,
            probes.bottomLine.middlePoint.x,
            probes.bottomLine.behindPoint.x
        };

        TileAttribute best = TileAttribute::None;

        for (double x : xs)
        {
            const auto a = classifyGroundAt_(x, probeY, includeOneWay, prevFootY);

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

    SweepVHit TileQueryService::sweepDown_(const Probes& probes, double dx, double dy) const
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
            const double probeX = x + dx;
            // Allow one-way here, but only when falling / moving downward.
            const bool includeOneWay = (dy > 0.0);
            const TileAttribute g = classifyGroundAt_(probeX, targetY, includeOneWay, includeOneWay ? std::optional<double>{curBottomY} : std::nullopt);
            if (g == TileAttribute::None)
            {
                continue;
            }

            const int row = static_cast<int>(std::floor(targetY / static_cast<double>(_ts)));
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

    std::array<double, 3> TileQueryService::collectBottomSampleXs_(const Probes& probes) const noexcept
    {
        return {
            probes.bottomLine.frontPoint.x,
            probes.bottomLine.middlePoint.x,
            probes.bottomLine.behindPoint.x
        };
    }

    TileAttribute TileQueryService::classifyGroundAt_(double x, double probeY, bool includeOneWay, std::optional<double> prevFootY) const
    {
        // Return true if we should treat the tile top as a "landing surface".
        // - If prevFootY is present: require crossing from above (or already on top).
        // - If prevFootY is absent: do NOT snap (avoid apex/idle false grounding).
        auto canLandOnTopSurface = [](const std::optional<double> prevFootY, const double curFootProbeY, const double topY, const double eps) -> bool
        {
            if (!prevFootY.has_value())
            {
                // No history => no snapping (prevents apex/idle one-frame grounding).
                return false;
            }
            const double prevY = *prevFootY;
            // Crossing: previous foot was above the top, current probe reached/passed the top.
            const bool crossedFromAbove = (prevY < topY - eps) && (curFootProbeY >= topY - eps);
            // Maintain: already standing on top within epsilon, and still not going above it.
            const bool alreadyOnTop = (std::abs(prevY - topY) <= eps) && (curFootProbeY >= topY - eps);
            return crossedFromAbove || alreadyOnTop;
        };

        const auto below = attrAt_(x, probeY);

        // Solid tile is always ground.
        if (Has(below, TileAttribute::Solid))
        {
            return TileAttribute::Solid;
        }

        // Compute the top Y of the tile row containing probeY.
        const int row = static_cast<int>(std::floor(probeY / static_cast<double>(_ts)));
        const double topY = static_cast<double>(row * _ts);

        constexpr double eps = SystemConfig::kEpsilon;

        // "Top surface" is valid only if the space just above is empty.
        const auto above = attrAt_(x, topY - eps);
        const bool hasEmptyAbove = Has(above, TileAttribute::Empty);

        // Ladder top: landable only on its top edge, and only when crossing from above.
        if (Has(below, TileAttribute::Ladder))
        {
            if (hasEmptyAbove && canLandOnTopSurface(prevFootY, probeY, topY, eps))
            {
                return TileAttribute::Ladder; // LadderTop as ground
            }
            return TileAttribute::None;
        }

        // One-way platform: landable only on its top edge, and only when crossing from above.
        if (includeOneWay && Has(below, TileAttribute::OneWayPlatform))
        {
            if (hasEmptyAbove && canLandOnTopSurface(prevFootY, probeY, topY, eps))
            {
                return TileAttribute::Solid; // Treat as floor when landing on its top.
            }
            return TileAttribute::None;
        }

        return TileAttribute::None;
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

    SweepVHit TileQueryService::sweepUp_(const Probes& probes, double dx, double dy) const
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
            const TileAttribute hitAttr = classifyCeilingAt_(x + dx, targetTopY);
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
        using AdjacentPolicy = world::stage::AdjacentPolicy;

        const int pageW = SystemConfig::kTileCountX * _ts;
        const int pageH = SystemConfig::kTileCountY * _ts;

        const auto resolvedPage = _grid.ResolvePageIndexFromWorldPos({ worldX, worldY });

        // No resolved page: out-of-bounds handling
        if (!resolvedPage)
        {
            const auto currOrigin = _grid.GetPageWorldOrigin(static_cast<int>(_currPage)); // {x,y}
            if (!currOrigin)
            {
                // Currently page origin is also unknown, so conservatively stop.
                return TileAttribute::Solid;
            }
            const double left = currOrigin->x;
            const double right = currOrigin->x + static_cast<double>(pageW);
            const double top = currOrigin->y;
            const double bottom = currOrigin->y + static_cast<double>(pageH);

            const bool outLeft = (worldX < left);
            const bool outRight = (worldX >= right);
            const bool outTop = (worldY < top);
            const bool outBottom = (worldY >= bottom);

            // Out of bounds to the right
            if (outRight)
            {
                const auto nx = _graph.AdjacentPageX(_currPage, +1, AdjacentPolicy::AnyConnection);
                return nx ? TileAttribute::Empty : TileAttribute::Solid;
            }

            // Out of bounds to the left
            if (outLeft)
            {
                const auto nx = _graph.AdjacentPageX(_currPage, -1, AdjacentPolicy::AnyConnection);
                return nx ? TileAttribute::Empty : TileAttribute::Solid;
            }

            // Out of bounds to the bottom (Death zone hack)
            if (outBottom)
            {
                const auto ny = _graph.AdjacentPageY(_currPage, +1, AdjacentPolicy::AnyConnection);
                return ny ? TileAttribute::Empty : TileAttribute::InstantDeath; // HACK: Death zone when falling out of bounds!
            }

            // Out of bounds to the top (special space where going off-screen does not cause failure)
            if (outTop)
            {
                const auto up = _graph.AdjacentPageY(_currPage, -1, AdjacentPolicy::AnyConnection);
                return up ? TileAttribute::Solid : TileAttribute::Empty;
                // If there is no page above, allow void (Empty)
            }

            // Conservatively stop if unable to determine at corners, etc.
            return TileAttribute::Solid;
        }

        // If resolvedPage is obtained, sample as usual.
        const std::size_t page = static_cast<std::size_t>(*resolvedPage);

        const int gx = PageGridIndex::FloorDiv(worldX, static_cast<double>(pageW));
        const int gy = PageGridIndex::FloorDiv(worldY, static_cast<double>(pageH));

        const double x = worldX - static_cast<double>(gx * pageW);
        const double y = worldY - static_cast<double>(gy * pageH);

        const int tx = std::clamp(static_cast<int>(x) / _ts, 0, static_cast<int>(SystemConfig::kTileCountX) - 1);
        const int ty = std::clamp(static_cast<int>(y) / _ts, 0, static_cast<int>(SystemConfig::kTileCountY) - 1);

        // Sample the tile attribute from the tile map provider.
        return _map.SampleTileAttributeOnPage(page, tx, ty);
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