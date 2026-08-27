//==============================================================================
// 
//  Project: mm2hack
//  GroundBaseState.h
// 
//  A avatar behavior base class for ground-based states.
// 
//==============================================================================
#pragma once

#include "apps/world/entity/avatar/IPlayerState.h"

#include <optional>
#include <string>
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    // Base class for ground-based action player states
    class GroundBaseState : public IPlayerState
    {
    protected:
        // Run transitions and physics shared by all ground states.
        [[nodiscard]] std::optional<AvatarStatus> UpdateGroundState(
            PlayerContext& cx,
            StateProvider* in,
            const PlayerTuning& t,
            GroundMoveIntent intent);

        // Apply directional input while only animation is advancing.
        void UpdateFacing(AnimeContext& ax, StateProvider* in) const noexcept;

    private:
        void groundPipeline_(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, GroundMoveIntent intent);
        [[nodiscard]] bool tryEnterLadderFromGround_(PlayerContext& cx, StateProvider* in) const;
        [[nodiscard]] bool tryEnterSliding_(PlayerContext& cx, StateProvider* in) const;

        const std::wstring kClassName{ L"GroundBaseState" };
    };
}
