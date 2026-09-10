//==============================================================================
// 
//  Project: mm2hack
//  IPhase.h
// 
//  Interface for phases within a scene.
// 
//==============================================================================
#pragma once

namespace mm2hack::apps::resources::parameters
{
    class Parameters;
}

namespace mm2hack::apps::scenes::phases
{
    struct PhaseResult;

    // Interface for phases within a scene
    class IPhase
    {
    public:
        virtual ~IPhase() = default;
        // Initializes the phase with given parameters
        virtual void Initialize(const resources::parameters::Parameters& params) = 0;
        // Returns PhaseResult indicating the outcome of the update
        virtual PhaseResult Update() = 0;
        // Renders the world elements
        virtual void RenderWorld() = 0;
        // Renders the overlay elements
        virtual void RenderOverlay() = 0;
        // Enables or disables the operate phase
        virtual void SetEnableOperatePhase(bool enable) = 0;
        // Gets whether the operate phase is enabled
        virtual bool GetEnableOperatePhase() const = 0;
    };
}