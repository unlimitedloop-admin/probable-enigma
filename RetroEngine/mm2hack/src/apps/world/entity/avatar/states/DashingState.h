//==============================================================================
//
//  Project: mm2hack
//  DashingState.h
//
//  Low-profile, two-speed ground dash state.
//
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/IPlayerState.h"

#include <cstdint>
#include <string>
#include "apps/systems/physics/Probes.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    class DashingState final : public IPlayerState
    {
    public:
        [[nodiscard]] AvatarStatus Id() const noexcept override;
        void OnEnter(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) override;
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double dt) override;
        void TickAnimationOnly(AnimeContext& ax, StateProvider* in, const PlayerTuning& t, double dt) override;

    private:
        const std::wstring kClassName{ L"DashingState" };

        [[nodiscard]] Probes makeDashProbes_(const PlayerContext& cx, const PlayerTuning& t) const;
        [[nodiscard]] bool hasStandingClearance_(const PlayerContext& cx, const PlayerTuning& t, double dx) const;
        [[nodiscard]] bool isStartup_(const PlayerTuning& t) const noexcept;
        void setPose_(PlayerContext& cx, const PlayerTuning& t) const noexcept;
        void setPose_(AnimeContext& ax, const PlayerTuning& t) const noexcept;

        std::uint8_t _elapsed_frames{ 0 };
        AvatarDirection _direction{ AvatarDirection::Right };
    };
}
