//==============================================================================
// 
//  Project: mm2hack
//  IPhaseHost.h
// 
//  Interface for a phase controller that manages phase transitions.
// 
//==============================================================================
#pragma once

#include <string>

#include "apps/resources/parameters/Parameters.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/world/session/PlayerSession.h"

namespace mm2hack::apps::scenes::phases
{
    // Implemented by Scene (e.g., TowerStage).
    // Phase never queues phases directly; it only requests a transition.
    class IPhaseHost
    {
    public:
        virtual ~IPhaseHost() = default;
        // Request a phase transition with the given key, fade plan, and parameters
        virtual void RequestTransition(const std::wstring& next_key, const PhaseFadePlan& plan, const resources::parameters::Parameters& params) = 0;
        // The player's lives etc., which outlive the phase (a miss rebuilds it).
        // Anything that grants a life -- 1UP item, score threshold -- goes
        // through AddLives() on this.
        virtual world::session::PlayerSession& Session() noexcept = 0;
    };
}
