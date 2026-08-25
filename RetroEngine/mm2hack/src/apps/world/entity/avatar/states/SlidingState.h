//==============================================================================
//
//  Project: mm2hack
//  SlidingState.h
//
//  Low-profile, fixed-speed ground movement state.
//
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/IPlayerState.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    class SlidingState final : public IPlayerState
    {
    public:
        [[nodiscard]] AvatarStatus Id() const noexcept override;
        void OnEnter(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) override;
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double dt) override;
        void TickAnimationOnly(AnimeContext& ax, StateProvider* in, const PlayerTuning& t, double dt) override;

    private:
        [[nodiscard]] Probes makeSlidingProbes_(const PlayerContext& cx, const PlayerTuning& t) const;
        [[nodiscard]] bool hasStandingClearance_(const PlayerContext& cx, const PlayerTuning& t) const;
        void setPose_(PlayerContext& cx) const noexcept;
        void setPose_(AnimeContext& ax) const noexcept;

        std::uint8_t _elapsed_frames{ 0 };
    };
}
