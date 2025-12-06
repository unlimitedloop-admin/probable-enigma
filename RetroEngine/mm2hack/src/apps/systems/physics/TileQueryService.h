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
        SweepHit SweepVertical(const Probes& probes, double dy) const override;
        // Returns true if the feet are considered "ground-like" (floor or ladder top special case)
        bool IsGroundLike(const AvatarDirection direction, const Probes& probes, double dy) const override;
        // Returns true if special handling for being at the "top" of a ladder is needed
        bool IsLadderTop(const AvatarDirection direction, const Probes& probes, double dy) const override;

    private:
        bool hasBlockUnderfoot(const AvatarDirection direction, const Probes& probes, double dy) const;
        bool isLadderTopUnderfoot(const AvatarDirection direction, const Probes& probes, double dy) const;

        TileAttribute attrAt(double worldX, double worldY) const;

    private:
        const std::wstring kClassName{ L"TileQueryService" };

        const ITileMapProvider& _map;
        int _ts{ SystemConfig::kTileSize };  // Tile size
    };
}