//==============================================================================
//
//  Project: mm2hack
//  PlayerFrameOutput.h
//
//  Events and commands emitted by player processing for external consumers.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "apps/world/entity/common/SpawnSplashEffectCommand.h"

namespace mm2hack::apps::world::entity::avatar
{
    // Semantic player events interpreted by the surrounding gameplay phase
    enum class PlayerEventType : std::uint8_t
    {
        EnteredWater,
        FiredRockBuster,
        Landed,
        IntroLanded
    };

    // A player event emitted during gameplay processing
    struct PlayerEvent final
    {
        PlayerEventType type{};
    };

    // All externally consumed output accumulated since the previous take
    struct PlayerFrameOutput final
    {
        std::vector<PlayerEvent> events{};
        std::optional<common::SpawnProjectileCommand> projectile{};
        std::optional<common::SpawnSplashEffectCommand> splashEffect{};

        void PushEvent(PlayerEventType type)
        {
            events.push_back(PlayerEvent{ type });
        }
    };
}
