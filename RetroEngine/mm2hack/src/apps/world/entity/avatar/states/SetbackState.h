//==============================================================================
//
//  Project: mm2hack
//  SetbackState.h
//
//  Forced, uncontrollable knockback reaction played on taking damage.
//  Entered externally via PlayerStateMachine::ForceTransition() (not a normal
//  player-initiated transition) -- see PlayerEntity::Update().
//
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/IPlayerState.h"

#include <cstdint>
#include <string>

#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    class SetbackState final : public IPlayerState
    {
    public:
        [[nodiscard]] AvatarStatus Id() const noexcept override;
        void OnEnter(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) override;
        AvatarStatus Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double dt) override;

        [[nodiscard]] std::uint8_t ElapsedFrames() const noexcept { return _elapsed_frames; }
        [[nodiscard]] bool Airborne() const noexcept { return _airborne; }
        [[nodiscard]] bool Rising() const noexcept { return _rising; }
        bool RestoreState(std::uint8_t elapsed_frames, bool airborne, bool rising) noexcept;

    private:
        const std::wstring kClassName{ L"SetbackState" };

        // Knockback tuning. Hardcoded here (not PlayerTuning) -- self-contained
        // to this one state, and IPlayerState's fixed method signatures have no
        // room for a second tuning parameter; mirrors DashingState::
        // makeDashProbes_()'s own local hardcoded offsets.
        static constexpr std::uint8_t kDurationFrames{ 28 };
        static constexpr double kGroundRetreatPxPerFrame{ 0.5 };
        static constexpr double kAirRisePxPerFrame{ 1.0 };
        static constexpr double kAirFallPxPerFrame{ 2.0 };
        // Below this vel.y (more negative = faster upward), the hit is judged
        // "still clearly ascending from a jump" -- rise further; otherwise fall.
        static constexpr double kAirRiseThreshold{ -0x02.00p0 };
        static constexpr std::uint8_t kBodyFramesPerTile{ 2 };
        static constexpr std::uint8_t kEffectFramesPerTile{ 8 };
        // Left-facing offsets, matching STile::ToTheLeft(40) for the body tile
        // and the art's own +4 convention for the effect tile.
        static constexpr int kEffectLeftOffset{ 4 };

        std::uint8_t _elapsed_frames{ 0 };
        bool _airborne{ false };
        bool _rising{ false };  // Only meaningful when _airborne
    };
}
