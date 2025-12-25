//==============================================================================
// 
//  Project: mm2hack
//  TileQueryService.h
// 
//  Service for querying tile attributes and terrain information.
// 
//==============================================================================
#pragma once

#include "apps/systems/physics/ITerrainProbe.h"

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ITileMapProvider.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/stage/RoomGraphAdapter.h"
#include "config/SystemConfig.h"
#include "Probes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    using config::SystemConfig;

    using foundation::math::Vec2;
    using world::stage::RoomGraphAdapter;

    class PageGridIndex;

    // Tile query service implementation
    class TileQueryService final : public ITerrainProbe
    {
    public:
        explicit TileQueryService(const ITileMapProvider& map, const RoomGraphAdapter& graph, const PageGridIndex& grid, int tilePx)
            : _map(map), _graph(graph), _grid(grid), _ts(tilePx)
        {
        }

        // Set the current page index for tile queries
        void SetCurrentPage(std::size_t pageIndex) noexcept override { _currPage = pageIndex; }

        // H-sweep: If moving dx, will it hit something?
        SweepHHit SweepHorizontal(const Probes& probes, double dx, bool airFlag = false) const override;
        // V-sweep: If moving dy, will it hit something?
        SweepVHit SweepVertical(const Probes& probes, Vec2 v) const override;
        // Returns true if the feet are considered "ground-like" (floor or ladder top special case)
        bool IsGroundLike(const AvatarDirection direction, const Probes& probes, double dy) const override;
        // Returns true if special handling for being at the "top" of a ladder is needed
        bool IsLadderTop(const AvatarDirection direction, const Probes& probes, double dy) const override;
        // Resolve horizontal overlap with Solid tiles
        OverlapXFix ResolveOverlapX(const Probes& p, const double parity) const override;

        // Get direct tile attribute at world position (=> attrAt_)
        TileAttribute AttributeAt(Vec2 pos) const override;
        TileAttribute AttributeAt(double worldX, double worldY) const override;

    private:
        // Check for wall collision using side probes
        bool probeWall_(const Probes& probes, TileAttribute& outAttr, double peekX = 1.0) const;
        // Sweeps rightwards to check for collisions with the terrain
        SweepHHit sweepRight_(const Probes& probes, double dx, bool airFlag = false) const;
        // Sweeps leftwards to check for collisions with the terrain
        SweepHHit sweepLeft_(const Probes& probes, double dx, bool airFlag = false) const;

        // Check for ground collision using bottom probes
        bool probeGround_(const Probes& probes, TileAttribute& outAttr) const;
        // Sweeps downwards to check for collisions with the terrain
        SweepVHit sweepDown_(const Probes& probes, double dx, double dy) const;

        // Check to see if there are any blocks at underfoot meeting the criteria
        bool hasBlockUnderfoot_(const AvatarDirection direction, const Probes& probes, double dy) const;
        // Collect X positions for bottom probes based on avatar direction
        void collectBottomSampleXs_(const AvatarDirection direction, const Probes& probes, double(&outXs)[3]) const noexcept;
        // Check if the avatar is standing on top of a ladder
        bool isLadderTopUnderfoot_(const AvatarDirection direction, const Probes& probes, double dy) const;

        // Check just above the head line for ceiling collision (dy==0 stability / "bonk" checks)
        bool probeCeiling_(const Probes& probes, TileAttribute& outAttr, double peekY = 1.0) const;
        // Sweep upwards to check for collisions with the terrain
        SweepVHit sweepUp_(const Probes& probes, double dx, double dy) const;

        // Attribute sampling
        TileAttribute attrAt_(double worldX, double worldY) const;

        // Classify tile attributes at specific probe points
        TileAttribute classifyGroundAt_(double x, double probeY, bool includeOneWay) const;
        // Classify ceiling tile attributes at specific probe points, only Solid collides from below
        TileAttribute classifyCeilingAt_(double x, double probeY) const;
        // Classify wall tile attributes at specific probe points, only Solid collides from sides
        TileAttribute classifyWallAt_(double x, double y) const;

    private:
        const std::wstring kClassName{ L"TileQueryService" };

        const ITileMapProvider& _map;   // Tile map provider
        const RoomGraphAdapter& _graph; // Room graph adapter
        const PageGridIndex& _grid;     // Page grid index for spatial mapping
        int _ts{};                      // Tile size

        std::size_t _currPage{ 0 };     // Current page index for tile queries
    };
}