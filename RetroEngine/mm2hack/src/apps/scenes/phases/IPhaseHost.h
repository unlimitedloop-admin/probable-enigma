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
    };
}
