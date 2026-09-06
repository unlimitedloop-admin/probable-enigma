#include "pch.h"

#include "PlayerEntityState.h"

#include <cmath>
#include <cstdlib>
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
    namespace
    {
        constexpr double kMaximumCoordinate = 1'000'000.0;

        bool IsCoordinate(double value) noexcept
        {
            return std::isfinite(value) && std::abs(value) <= kMaximumCoordinate;
        }

        bool IsTexture(std::int32_t value) noexcept
        {
            return value >= -32'768 && value <= 65'535;
        }

        bool IsDirection(systems::scrolling::atomic::PageScroll::Dir direction) noexcept
        {
            using Direction = systems::scrolling::atomic::PageScroll::Dir;
            return direction >= Direction::Right && direction <= Direction::Up;
        }
    }

    bool PlayerEntityState::Save(core::save::StateWriter& writer) const
    {
        if (!IsValid() || !kinematic.Save(writer) ||
            !writer.WriteBool(collidable) || !writer.WriteBool(on_ground) ||
            !writer.WriteI32(static_cast<std::int32_t>(facing)) ||
            !writer.WriteI32(base_texture) || !writer.WriteI32(attack_texture) ||
            !locomotion.Save(writer) || !attack.Save(writer) ||
            !writer.WriteBool(rock_buster.visible) ||
            !writer.WriteI32(rock_buster.armTexture) ||
            !writer.WriteF64(rock_buster.offset.x) ||
            !writer.WriteF64(rock_buster.offset.y) ||
            !environment.Save(writer) ||
            !writer.WriteBool(intro.active) ||
            !writer.WriteU8(static_cast<std::uint8_t>(intro.phase)) ||
            !writer.WriteF64(intro.timer) ||
            !writer.WriteF64(intro.offsetPos.x) ||
            !writer.WriteF64(intro.offsetPos.y) ||
            !writer.WriteF64(intro.destPos.x) ||
            !writer.WriteF64(intro.destPos.y) ||
            !writer.WriteF64(intro.dropDuration) ||
            !animation.Save(writer) ||
            !writer.WriteBool(jump_buffered) || !writer.WriteBool(dash_buffered) ||
            !writer.WriteF64(view_bounds.leftX) ||
            !writer.WriteF64(view_bounds.rightX) ||
            !writer.WriteF64(view_bounds.topY) ||
            !writer.WriteF64(view_bounds.bottomY) ||
            !writer.WriteF64(page_origin.x) || !writer.WriteF64(page_origin.y) ||
            !writer.WriteU32(scroll_page_index) ||
            !writer.WriteBool(pending_scroll.has_value()))
        {
            return false;
        }

        if (pending_scroll.has_value() &&
            (!writer.WriteBool(pending_scroll->available) ||
             !writer.WriteU8(static_cast<std::uint8_t>(pending_scroll->dir)) ||
             !writer.WriteF64(pending_scroll->carryTotalPx)))
        {
            return false;
        }

        return writer.WriteBool(fixed_scroll_available) &&
            writer.WriteU8(static_cast<std::uint8_t>(charge.phase)) &&
            writer.WriteU32(charge.frames) && writer.WriteU32(charge.phaseFrames);
    }

    bool PlayerEntityState::Load(core::save::StateReader& reader)
    {
        PlayerEntityState loaded{};
        std::int32_t encoded_facing{};
        std::uint8_t encoded_intro_phase{};
        bool has_pending_scroll{};
        if (!loaded.kinematic.Load(reader) ||
            !reader.ReadBool(loaded.collidable) ||
            !reader.ReadBool(loaded.on_ground) ||
            !reader.ReadI32(encoded_facing) ||
            !reader.ReadI32(loaded.base_texture) ||
            !reader.ReadI32(loaded.attack_texture) ||
            !loaded.locomotion.Load(reader) ||
            !loaded.attack.Load(reader) ||
            !reader.ReadBool(loaded.rock_buster.visible) ||
            !reader.ReadI32(loaded.rock_buster.armTexture) ||
            !reader.ReadF64(loaded.rock_buster.offset.x) ||
            !reader.ReadF64(loaded.rock_buster.offset.y) ||
            !loaded.environment.Load(reader) ||
            !reader.ReadBool(loaded.intro.active) ||
            !reader.ReadU8(encoded_intro_phase) ||
            !reader.ReadF64(loaded.intro.timer) ||
            !reader.ReadF64(loaded.intro.offsetPos.x) ||
            !reader.ReadF64(loaded.intro.offsetPos.y) ||
            !reader.ReadF64(loaded.intro.destPos.x) ||
            !reader.ReadF64(loaded.intro.destPos.y) ||
            !reader.ReadF64(loaded.intro.dropDuration) ||
            !loaded.animation.Load(reader) ||
            !reader.ReadBool(loaded.jump_buffered) ||
            !reader.ReadBool(loaded.dash_buffered) ||
            !reader.ReadF64(loaded.view_bounds.leftX) ||
            !reader.ReadF64(loaded.view_bounds.rightX) ||
            !reader.ReadF64(loaded.view_bounds.topY) ||
            !reader.ReadF64(loaded.view_bounds.bottomY) ||
            !reader.ReadF64(loaded.page_origin.x) ||
            !reader.ReadF64(loaded.page_origin.y) ||
            !reader.ReadU32(loaded.scroll_page_index) ||
            !reader.ReadBool(has_pending_scroll))
        {
            return false;
        }

        loaded.facing = static_cast<AvatarDirection>(encoded_facing);
        loaded.intro.phase = static_cast<IntroPhase>(encoded_intro_phase);
        if (has_pending_scroll)
        {
            bool available{};
            std::uint8_t encoded_direction{};
            double carry{};
            if (!reader.ReadBool(available) ||
                !reader.ReadU8(encoded_direction) ||
                !reader.ReadF64(carry))
            {
                return false;
            }
            loaded.pending_scroll = systems::scrolling::atomic::FixedScrollRequest{
                available,
                static_cast<systems::scrolling::atomic::PageScroll::Dir>(encoded_direction),
                carry
            };
        }

        std::uint8_t encoded_charge_phase{};
        if (!reader.ReadBool(loaded.fixed_scroll_available) ||
            !reader.ReadU8(encoded_charge_phase) ||
            !reader.ReadU32(loaded.charge.frames) ||
            !reader.ReadU32(loaded.charge.phaseFrames))
        {
            return false;
        }
        loaded.charge.phase = static_cast<ChargePhase>(encoded_charge_phase);
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool PlayerEntityState::IsValid() const noexcept
    {
        const bool intro_valid =
            (intro.phase == IntroPhase::Falling ||
             intro.phase == IntroPhase::Landing ||
             intro.phase == IntroPhase::Done) &&
            std::isfinite(intro.timer) && intro.timer >= 0.0 && intro.timer <= 3'600.0 &&
            IsCoordinate(intro.offsetPos.x) && IsCoordinate(intro.offsetPos.y) &&
            IsCoordinate(intro.destPos.x) && IsCoordinate(intro.destPos.y) &&
            std::isfinite(intro.dropDuration) &&
            intro.dropDuration > 0.0 && intro.dropDuration <= 60.0;
        const bool bounds_valid =
            IsCoordinate(view_bounds.leftX) && IsCoordinate(view_bounds.rightX) &&
            IsCoordinate(view_bounds.topY) && IsCoordinate(view_bounds.bottomY) &&
            view_bounds.leftX <= view_bounds.rightX &&
            view_bounds.topY <= view_bounds.bottomY;
        const bool pending_valid = !pending_scroll.has_value() ||
            (pending_scroll->available && IsDirection(pending_scroll->dir) &&
             std::isfinite(pending_scroll->carryTotalPx) &&
             pending_scroll->carryTotalPx >= 0.0 &&
             pending_scroll->carryTotalPx <= kMaximumCoordinate);
        const bool charge_valid =
            charge.phase >= ChargePhase::Idle && charge.phase <= ChargePhase::Level2 &&
            charge.frames == attack.charge_frames &&
            charge.phaseFrames <= charge.frames &&
            ((charge.phase == ChargePhase::Level1 || charge.phase == ChargePhase::Level2) ||
             charge.phaseFrames == 0) &&
            (attack.charging ? charge.phase != ChargePhase::Idle
                             : charge.phase == ChargePhase::Idle);

        return kinematic.IsValid() &&
            (facing == AvatarDirection::Left || facing == AvatarDirection::Right) &&
            IsTexture(base_texture) && IsTexture(attack_texture) &&
            locomotion.IsValid() && attack.IsValid() &&
            IsTexture(rock_buster.armTexture) &&
            IsCoordinate(rock_buster.offset.x) && IsCoordinate(rock_buster.offset.y) &&
            environment.IsValid() && intro_valid && animation.IsValid() &&
            bounds_valid && IsCoordinate(page_origin.x) && IsCoordinate(page_origin.y) &&
            scroll_page_index <= 65'535 && pending_valid && charge_valid;
    }
}
