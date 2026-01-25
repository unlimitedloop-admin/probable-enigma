//==============================================================================
// 
//  Project: mm2hack
//  AttackActionState.h
// 
//  Provides attack action state handling for the avatar (attacking or not).
// 
//==============================================================================
#pragma once

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/common/SpawnProjectileCommand.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar
{
    // Minimal tuning values needed for attack projectile
    struct AttackTuning final
    {
        int attack_texture_add{ 10 };

        // Projectile
        double projectile_speed_px_per_sec{ 240.0 };
        foundation::math::Vec2 projectile_spawn_offset_px_right{ 12.0, -4.0 };
        foundation::math::Vec2 projectile_spawn_offset_px_left{ -12.0, -4.0 };

        int projectile_base_texture{ 200 }; // TODO: set your tile index
        int projectile_anim_frames{ 2 };
        double projectile_anim_fps{ 12.0 };
        double projectile_life_sec{ 1.2 };

        // Attack timing
        double attack_duration_sec{ 0.18 };
    };

    // Handles attack action state (attacking or not)
    class AttackActionState final
    {
        using StateProvider = core::assembly::StateProvider;

    public:
        AttackActionState() = default;

        // Returns texture_add (+10) while attacking and optional spawn command on attack start.
        common::SpawnProjectileCommand Update(
            PlayerContext& cx, StateProvider* in, const AttackTuning& tuning,
            bool can_spawn_projectile,
            double dt);

        // Is currently attacking
        [[nodiscard]] bool IsAttacking() const noexcept;
        // Tick animation only (no state update)
        [[nodiscard]] void TickAnimationOnly(AnimeContext& ax, const AttackTuning& tuning, double /*dt*/) const noexcept;

        // Get rock buster offset for given texture and facing direction
        Vec2 GetRockBusterOffset(int texture, AvatarDirection facingLR) const noexcept;

    private:
        void restartAttackPose_() noexcept; // Start attack action

    private:
        const std::wstring kClassName{ L"AttackActionState" };

        bool _is_attacking{ false };        // Whether currently attacking
        double _pose_time_sec{ 10.0 };      // Time spent in shot pose
    };
}