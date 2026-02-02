//==============================================================================
// 
//  Project: mm2hack
//  AttackActionState.h
// 
//  Provides attack action state handling for the avatar (attacking or not).
// 
//==============================================================================
#pragma once

#include <optional>
#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    // Rock Buster drawing info
    struct RockBusterDrawInfo final
    {
        bool visible{ false };
        int armTexture{ 0 };   // Original texture index
        Vec2 offset{};
    };

    // Result of action update for player state handlers
    struct ActionUpdateResult final
    {
        int textureAdd{ 0 };
        bool lockClimbMove{ false };
        std::optional<common::SpawnProjectileCommand> spawnProjectile{};
        RockBusterDrawInfo rockBuster{};
    };

    // Rock Buster tuning values
    struct RockBusterTuning final
    {
        int arm_texture_right{ 10 };
        int arm_texture_left{ 50 };
    };

    // Minimal tuning values needed for attack projectile
    struct AttackTuning final
    {
        int facingOffsetRight{ 0 };
        int facingOffsetLeft{ 1 };

        int attackTextureAdd{ 10 };

        // Projectile
        double projectileSpeedPxPerSec{ 240.0 };
        foundation::math::Vec2 projectileSpawnOffsetPxRight{ 32.0, 5.0 };
        foundation::math::Vec2 projectileSpawnOffsetPxLeft{ -16.0, 5.0 };

        int rockBusterTexture{ 0 };
        int projectileAnimFrames{ 1 };
        double projectileAnimFps{ 12.0 };
        double projectileLifeSec{ -1.0 };

        // Attack timing
        double attackDurationSec{ 0.18 };
    };

    // Handles attack action state (attacking or not)
    class AttackActionState final
    {
        using StateProvider   = core::assembly::StateProvider;
        using SpriteManagerId = rendering::sprite::SpriteManager::Id;

    public:
        AttackActionState(SpriteManagerId id) : _id(id) {}

        SpriteManagerId Id() const noexcept { return _id; }

        void PreUpdate(PlayerContext& cx, StateProvider* in, bool can_spawn) noexcept;
        ActionUpdateResult PostUpdate(PlayerContext& cx, StateProvider* in, const AttackTuning& tuning, double dt);

        // Is currently attacking
        [[nodiscard]] bool IsAttacking() const noexcept;
        // Tick animation only (no state update)
        [[nodiscard]] void TickAnimationOnly(AnimeContext& ax, const AttackTuning& tuning, double dt, RockBusterDrawInfo& out_rb) const noexcept;

    private:
        void restartAttackPose_() noexcept; // Start attack action
        void finishAttackPose_() noexcept;  // Finish attack action

    private:
        const std::wstring kClassName{ L"AttackActionState" };

        SpriteManagerId _id{};              // Weapon sprite id
        bool _is_attacking{ false };        // Whether currently attacking
        double _pose_time_sec{ 10.0 };      // Time spent in shot pose
        RockBusterTuning rb_tuning{};       // Rock Buster tuning

        bool _fire_requested{ false };      // Whether fire button was requested
    };
}