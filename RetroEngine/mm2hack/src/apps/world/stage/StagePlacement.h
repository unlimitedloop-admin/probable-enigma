//==============================================================================
//
//  Project: mm2hack
//  StagePlacement.h
//
//  What a stage's .def (BD-006 stage definition data) says about WHERE
//  things go: the player's start point and every placed enemy. Deliberately
//  nothing about what an enemy can do -- that's the kind's own definition
//  (EnemyAbilities in data/enemies/<KIND>.json).
//
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "apps/foundation/math/CoordinateTypes.h"

namespace mm2hack::apps::world::stage
{
    // When a placed enemy may appear again after its first appearance. Every
    // placement is reset when the stage restarts (e.g. after a miss).
    enum class EnemyRespawnPolicy : std::uint8_t
    {
        // Each time the placement point scrolls out of view and back in
        // (with no live instance around), even if it was defeated -- the
        // classic behavior.
        Always,
        // Like Always, but never again once defeated.
        UntilDefeated,
        // Only the first time the placement point comes into view.
        Once
    };

    enum class PlacementFacing : std::uint8_t
    {
        TowardPlayer,
        Left,
        Right
    };

    // One "type": "enemy" entry of the .def's "entities".
    struct EnemyPlacement final
    {
        std::string id;                         // Unique within the stage
        int room_id{ 0 };
        // Room-local pixels (0-255, 0-239), anchored at the enemy's feet,
        // horizontally centered -- independent of the sprite's size.
        foundation::math::Vec2 local_pos{};
        std::string kind;                       // Enemy definition "id", e.g. "metall"
        std::string palette{ "default" };       // Palette preset id; unknown ones fall back to the default
        PlacementFacing facing{ PlacementFacing::TowardPlayer };
        EnemyRespawnPolicy respawn{ EnemyRespawnPolicy::Always }; // Already resolved against the room's default
        bool despawn_offscreen{ true };         // Removed once it leaves the view
    };

    struct PlayerStart final
    {
        int room_id{ 0 };
        foundation::math::Vec2 local_pos{};     // Room-local pixels, same anchor as EnemyPlacement
    };

    struct StageDefinitionData final
    {
        std::optional<PlayerStart> start;
        std::vector<EnemyPlacement> enemies;
    };
}
