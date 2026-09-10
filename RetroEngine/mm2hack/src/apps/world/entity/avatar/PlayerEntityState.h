//==============================================================================
//
//  Project: mm2hack
//  PlayerEntityState.h
//
//  Logical, resource-independent snapshot for PlayerEntity.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <optional>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/world/entity/common/AnimeStepper.h"
#include "apps/world/entity/EntityBase.h"
#include "AvatarStatus.h"
#include "core/save/StateIO.h"
#include "PlayerContext.h"
#include "PlayerEnvironmentController.h"
#include "PlayerFrameOutput.h"
#include "PlayerParams.h"
#include "PlayerStateMachine.h"
#include "states/AttackActionState.h"

namespace mm2hack::apps::world::entity::avatar
{
    struct PlayerEntityState final
    {
        EntityKinematicState kinematic{};
        bool collidable{ true };
        bool on_ground{};
        AvatarDirection facing{ AvatarDirection::Right };
        std::int32_t base_texture{};
        std::int32_t attack_texture{};
        PlayerStateMachineState locomotion{};
        states::AttackActionSnapshot attack{};
        states::RockBusterDrawInfo rock_buster{};
        PlayerEnvironmentState environment{};
        IntroDropState intro{};
        common::AnimeStepperState animation{};
        bool jump_buffered{};
        bool dash_buffered{};
        WorldBounds view_bounds{};
        foundation::math::Vec2 page_origin{};
        std::uint32_t scroll_page_index{};
        std::optional<systems::scrolling::atomic::FixedScrollRequest> pending_scroll{};
        bool fixed_scroll_available{};
        ChargeStatus charge{};

        bool Save(core::save::StateWriter& writer) const;
        bool Load(core::save::StateReader& reader);
        [[nodiscard]] bool IsValid() const noexcept;
    };
}
