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
#include "config/SystemConfig.h"
#include "Probes.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    using config::SystemConfig;
    using foundation::math::Vec2;

    // Tile query service implementation
    class TileQueryService final : public ITerrainProbe
    {
    public:
        explicit TileQueryService(const ITileMapProvider& map)
            : _map(map), _ts(SystemConfig::kTileSize) {}

        // V-sweep: If moving dy, will it hit something?
        SweepVHit SweepVertical(const Probes& probes, double dy) const override;
        // Returns true if the feet are considered "ground-like" (floor or ladder top special case)
        bool IsGroundLike(const AvatarDirection direction, const Probes& probes, double dy) const override;

    private:
        // Classify the ground tile attribute at the given position
        TileAttribute classifyGroundAt_(double x, double probeY, bool includeOneWay) const;
        // Collect bottom sample X coordinates based on avatar direction and probes
        void collectBottomSampleXs_(const AvatarDirection direction, const Probes& probes, double(&outXs)[3]) const noexcept;
        // Check if there is a block underfoot
        bool hasBlockUnderfoot_(const AvatarDirection direction, const Probes& probes, double dy) const;
        // Check if the avatar is at the top of a ladder underfoot
        bool isLadderTopUnderfoot_(const AvatarDirection direction, const Probes& probes, double dy) const;
        // Get the tile attribute at the specified world coordinates
        TileAttribute attrAt_(double worldX, double worldY) const;
        // Probe the ground and return its attribute
        bool probeGround_(const Probes& probes, TileAttribute& outAttr) const;
        // Sweep downwards and return the hit information
        SweepVHit sweepDown_(const Probes& probes, double dy) const;

    private:
        const std::wstring kClassName{ L"TileQueryService" };

        const ITileMapProvider& _map;
        int _ts{ SystemConfig::kTileSize };     // Tile size
    };
}