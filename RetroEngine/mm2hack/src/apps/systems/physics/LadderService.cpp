#include "pch.h"

#include "LadderService.h"

#include "apps/foundation/math/CoordinateTypes.h"
#include "ITerrainProbe.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    LadderService::LadderService(const ITerrainProbe& terrain)
        : _terrain(terrain)
    {
    }

    int LadderService::toTileIndex_(double v)
    {
        // world -> tile index (0-based)
        // NOTE: Like other tile index calculations, this uses floor division.
        return static_cast<int>(v / config::SystemConfig::kTileSize);
    }

    double LadderService::toTileCenter_(int tileIndex)
    {
        return (static_cast<double>(tileIndex) + 0.5) * config::SystemConfig::kTileSize;
    }

    bool LadderService::CanGrabAt(const Vec2& worldPos) const
    {
        const auto attr = _terrain.AttributeAt(worldPos);
        return Has(attr, TileAttribute::Ladder);    // attr == TileAttribute::Laddering
    }

    std::optional<Vec2> LadderService::TryGetCenterXAt(const Vec2& worldPos) const
    {
        if (!CanGrabAt(worldPos))
        {
            return std::nullopt;
        }

        const int tile_x = toTileIndex_(worldPos.x);
        const int tile_y = toTileIndex_(worldPos.y);

        Vec2 snap{};
        snap.x = toTileCenter_(tile_x);
        snap.y = worldPos.y;

        return snap;
    }
}
