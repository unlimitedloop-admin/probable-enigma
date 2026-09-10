//==============================================================================
// 
//  Project: mm2hack
//  AttackActionState.h
// 
//  Provides attack action state handling for the avatar (attacking or not).
// 
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/rendering/sprite/SpriteManager.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerFrameOutput.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "core/assembly/StateProvider.h"
#include "core/save/StateIO.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    struct AttackActionSnapshot final
    {
        bool attacking{};
        double pose_time_seconds{};
        bool fire_requested{};
        bool charging{};
        std::uint32_t charge_frames{};
        common::ProjectileVisual requested_visual{ common::ProjectileVisual::Normal };

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };

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
        int facingOffsetLeft{ 32 };

        int attackTextureAdd{ 10 };

        // Projectile
        double projectileSpeedPxPerSec{ 240.0 };
        double chargeLevel1SpeedPxPerSec{ 300.0 };
        double chargeLevel2SpeedPxPerSec{ 360.0 };
        foundation::math::Vec2 projectileSpawnOffsetPxRight{ 32.0, 5.0 };
        foundation::math::Vec2 projectileSpawnOffsetPxLeft{ -16.0, 5.0 };

        int rockBusterTexture{ 0 };
        int projectileAnimFrames{ 1 };
        double projectileAnimFps{ 12.0 };
        double projectileLifeSec{ -1.0 };

        // Attack timing
        double attackDurationSec{ 0.18 };
        std::uint32_t level1ChargeFrames{ 20 };
        std::uint32_t level2ChargeFrames{ 200 };
    };

    // Handles attack action state (attacking or not)
    class AttackActionState final
    {
    public:
        AttackActionState(rendering::sprite::SpriteManager::Id id) : _id(id) {}

        void PreUpdate(PlayerContext& cx, core::assembly::StateProvider* in, bool can_spawn) noexcept;
        ActionUpdateResult PostUpdate(PlayerContext& cx, core::assembly::StateProvider* in, const AttackTuning& tuning, double dt);

        // Is currently attacking
        [[nodiscard]] bool IsAttacking() const noexcept;
        void Cancel() noexcept;
        // Tick animation only (no state update)
        void TickAnimationOnly(AnimeContext& ax, const AttackTuning& tuning, RockBusterDrawInfo& out_rb) const noexcept;
        [[nodiscard]] AttackActionSnapshot CaptureState() const noexcept;
        bool RestoreState(const AttackActionSnapshot& state) noexcept;

    private:
        void restartAttackPose_(bool request_normal_shot = false) noexcept; // Start attack action
        void finishAttackPose_() noexcept;          // Finish attack action
        [[nodiscard]] ChargePhase chargePhase_(const AttackTuning& tuning) const noexcept;
        void requestChargedShot_(const AttackTuning& tuning) noexcept;

    private:
        const std::wstring kClassName{ L"AttackActionState" };

        rendering::sprite::SpriteManager::Id _id{}; // Weapon sprite id
        bool _is_attacking{ false };                // Whether currently attacking
        double _pose_time_sec{ 10.0 };              // Time spent in shot pose
        RockBusterTuning rb_tuning{};               // Rock Buster tuning

        bool _fire_requested{ false };              // Whether fire button was requested
        bool _charging{ false };
        bool _reconcile_restored_charge_input{ false };
        bool _can_spawn{ false };
        std::uint32_t _charge_frames{ 0 };
        common::ProjectileVisual _requested_visual{ common::ProjectileVisual::Normal };
    };
}
