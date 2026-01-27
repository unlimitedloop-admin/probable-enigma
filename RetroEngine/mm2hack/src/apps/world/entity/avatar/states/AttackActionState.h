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
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar
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
        int facingOffsetLeft{ 40 };

        int attackTextureAdd{ 10 };

        // Projectile
        double projectileSpeedPxPerSec{ 240.0 };
        foundation::math::Vec2 projectileSpawnOffsetPxRight{ 12.0, -4.0 };
        foundation::math::Vec2 projectileSpawnOffsetPxLeft{ -12.0, -4.0 };

        int projectileBaseTexture{ 200 }; // TODO: set your tile index
        int projectileAnimFrames{ 2 };
        double projectileAnimFps{ 12.0 };
        double projectileLifeSec{ 1.2 };

        // Attack timing
        double attackDurationSec{ 0.18 };
    };

    // Handles attack action state (attacking or not)
    class AttackActionState final
    {
        using StateProvider = core::assembly::StateProvider;

    public:
        AttackActionState() = default;

        // Returns texture_add (+10) while attacking and optional spawn command on attack start.
        //ActionUpdateResult Update(PlayerContext& cx, StateProvider* in, const AttackTuning& tuning, bool can_spawn_projectile, double dt);

        void PreUpdate(PlayerContext& cx, StateProvider* in) noexcept;
        ActionUpdateResult PostUpdate(PlayerContext& cx, StateProvider* in, const AttackTuning& tuning, bool can_spawn, double dt);

        // Is currently attacking
        [[nodiscard]] bool IsAttacking() const noexcept;
        // Tick animation only (no state update)
        [[nodiscard]] void TickAnimationOnly(AnimeContext& ax, const AttackTuning& tuning, double dt, RockBusterDrawInfo& out_rb) const noexcept;

    private:
        void restartAttackPose_() noexcept; // Start attack action
        void finishAttackPose_() noexcept;  // Finish attack action

    private:
        const std::wstring kClassName{ L"AttackActionState" };

        bool _is_attacking{ false };        // Whether currently attacking
        double _pose_time_sec{ 10.0 };      // Time spent in shot pose
        RockBusterTuning rb_tuning{};       // Rock Buster tuning
    };
}