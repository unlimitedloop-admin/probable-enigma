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

#include <string>
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"

namespace mm2hack::apps::world::entity::avatar::states
{
    // Base class for ground-based action player states
    class GroundBaseState : public IPlayerState
    {
    protected:
        // Executes common ground processing; controlled to skip if not overridden (HoveringState etc...)
        void GroundPipeline(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, GroundMoveIntent intent);

        bool TryEnterLadderFromGround(PlayerContext& cx, StateProvider* in, const PlayerTuning& t) const;

    private:
        const std::wstring kClassName{ L"GroundBaseState" };
    };
}